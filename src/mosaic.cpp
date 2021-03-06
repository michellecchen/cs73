// mosaic.cpp (Part A & C)

#include "mosaic.h"
#include "utils.h"
#include <assert.h>
 
using namespace std;
using namespace Eigen;

// ------- PART A -------

// Recover the homography between 2 sets of corresponding points using least squares
// input: im1Points and im2Points are n-by-2 matrices holding the (x,y) locations
// of n point correspondences from the two images
// output: the recovered 3x3 homography matrix
Matrix3f computeHomography(const vector<vector<int>> im1Points, const vector<vector<int>> im2Points)
{
    if (im1Points.size() != im2Points.size() || im1Points.size() < 4) {
        throw MismatchedSizeException();
    }

    // form matrix A and vector b
    int n = im1Points.size();
    MatrixXf A = MatrixXf::Zero(2*n, 8); // 2n by 8 matrix
    VectorXf b(2*n); // 2n by 1 vector
	for (int i = 0; i < n; i++) {
        A(2*i, 0) = im1Points[i][0]; // x
        A(2*i, 1) = im1Points[i][1]; // y
        A(2*i, 2) = 1;
        A(2*i, 6) = - im1Points[i][0] * im2Points[i][0]; // -x*x'
        A(2*i, 7) = - im1Points[i][1] * im2Points[i][0]; // -y*x'

        A(2*i + 1, 3) = im2Points[i][0]; // x'
        A(2*i + 1, 4) = im2Points[i][1]; // y'
        A(2*i + 1, 5) = 1;
        A(2*i + 1, 6) = - im1Points[i][0] * im2Points[i][1]; // -x*y'
        A(2*i + 1, 7) = - im1Points[i][1] * im2Points[i][1]; // -y*y'

        b(2*i) = im2Points[i][0]; // x'
        b(2*i + 1) = im2Points[i][1]; // y'
    }
    
    // solve Ah = b and form homography matrix H
    VectorXf h = A.colPivHouseholderQr().solve(b); // solve using QR decomposition
    Matrix3f H(3, 3);
    H(0, 0) = h(0); H(0, 1) = h(1); H(0, 2) = h(2);
    H(1, 0) = h(3); H(1, 1) = h(4); H(1, 2) = h(5);
    H(2, 0) = h(6); H(2, 1) = h(7); H(2, 2) = 1;
    return H;
}

// Use bilinear interpolation to assign the value of a location from its neighboring pixel values
float interpolateLin(const FloatImage &im, float x, float y, int z, bool clamp)
{
	// interpolate along x for top left and top right
	int x1 = floor(x);
	int x2 = floor(x) + 1;
	int y1 = floor(y);
	float t = x - x1;
	float top = im.smartAccessor(x1, y1, z, clamp) * (1 - t) + im.smartAccessor(x2, y1, z, clamp) * t;

	// interpolate along x for bottom left and bottom right
	int y2 = floor(y) + 1;
	float bottom = im.smartAccessor(x1, y2, z, clamp) * (1 - t) + im.smartAccessor(x2, y2, z, clamp) * t;

	// interpolate along y for top and bottom
	t = y - y1;
	float output = top * (1 - t) + bottom * t;
	
	//return final float value
    return output;
}

// Compute the bounding box coordinates of an image after applying a given homography
// input: width and height of an image and a homography matrix H
// output: a vector storing [xmin, xmax, ymin, ymax]
vector<float> computeTransformedBBox(int width, int height, Matrix3f H) {
    Vector3f topLeft, topRight, bottomLeft, bottomRight;
    topLeft << 0, 0, 1;
    topRight << width, 0, 1;
    bottomLeft << 0, height, 1;
    bottomRight << width, height, 1;
    topLeft = H * topLeft;
    topRight = H * topRight;
    bottomLeft = H * bottomLeft;
    bottomRight = H * bottomRight;

    vector<float> result;
    result.push_back(min(topLeft[0] / topLeft[2], bottomLeft[0] / bottomLeft[2])); // xmin
    result.push_back(max(topRight[0] / topRight[2], bottomRight[0] / bottomRight[2])); // xmax
    result.push_back(min(topLeft[1] / topLeft[2], topRight[1] / topRight[2])); // ymin
    result.push_back(max(bottomLeft[1] / bottomLeft[2], bottomRight[1] / bottomRight[2])); // ymax
    return result;
}

// Warp an image using a given homography matrix and bilinear interpolation
// input: im is the input image to be warped and H is the homography matrix
// output: the warped image
FloatImage warpImage(const FloatImage &im, const Matrix3f H)
{
    vector<float> bbox1 = computeTransformedBBox(im.width(), im.height(), H);
    float tx = bbox1[0];
    float ty = bbox1[2];
    int width = (int) (bbox1[1] - bbox1[0]);
    int height = (int) (bbox1[3] - bbox1[2]);
    if (width <= 0 || height <= 0) {
        cout << "bad homography" << endl;
    }
    FloatImage output(width, height, im.channels());

	for (int i = 0; i < output.width(); i++) {
		for (int j = 0; j < output.height(); j++) {
			for (int c = 0; c < output.channels(); c++) {
                Vector3f imCoords;
                imCoords << i + tx, j + ty, 1;
                Vector3f outCoords = H.inverse() * imCoords; // use inverse warping
				float x = outCoords(0) / outCoords(2);
                float y = outCoords(1) / outCoords(2);
				output(i, j, c) = interpolateLin(im, x, y, c, false);
			}
		}
	}
	return output;
}

// Rectify an image given 2 sets of corresponding points
// input: im is the input image to be rectified, im1Points and im2Points are n-by-2 matrices
// holding the (x,y) locations of n point correspondences from the two images
// output: the warped image
FloatImage rectifyImage(const FloatImage &im, const std::vector<std::vector<int>> im1Points, const std::vector<std::vector<int>> im2Points)
{
    if (im1Points.size() != im2Points.size() || im1Points.size() < 4) {
        throw MismatchedSizeException();
    }
    Matrix3f H = computeHomography(im1Points, im2Points);
    return warpImage(im, H);
}

// Stitch 2 images together given a homography for warping the left image to the right image
// input: im1 and im2 are the left and right images to be stitched together,
// H is the homography matrix
// output: the stitched panorama image
FloatImage stitch(const FloatImage &im1, const FloatImage &im2, const Matrix3f H)
{
    FloatImage outIm1 = warpImage(im1, H);
    // outIm1.write("../data/output/part-A/warped-left-image.jpg");
    vector<float> bbox1 = computeTransformedBBox(im1.width(), im1.height(), H);
    float tx = bbox1[0];
    float ty = bbox1[2];
    int width = im2.width() - tx;
    int height = max((int) (bbox1[3] - bbox1[2]), (int) im2.height());

    // for linear/sigmoid blending
    int overlap_x1 = - tx;
    int overlap_x2 = bbox1[1] - tx;
    float mid = (overlap_x1 + overlap_x2) / 2.f;

    // combine warped left image and right image to form output image
    FloatImage output(width, height, im1.channels());
    for (int i = 0; i < output.width(); i++) {
		for (int j = 0; j < output.height(); j++) {
			for (int c = 0; c < output.channels(); c++) {
                float val1 = outIm1.smartAccessor(i, j, c, false);
                float val2 = im2.smartAccessor(i + tx, j + ty, c, false);
				output(i, j, c) = val1 + val2;
                if (val1 != 0 && val2 != 0) {
                    // output(i, j, c) = val1; // simple overlap
                    // float alpha = ((float) i - overlap_x1) / (overlap_x2 - overlap_x1); // linear blend
                    float alpha = 1 / (1 + exp(mid - i)); // sigmoid blend
                    output(i, j, c) = val1 * (1 - alpha) + val2 * alpha;
                }
			}
		}
	}
    return output;
}

// Stitch 2 images together given 2 sets of corresponding points (warp the left image)
// input: im1 and im2 are the left and right images to be stitched together,
// im1Points and im2Points are n-by-2 matrices holding the (x,y) locations
// of n point correspondences
// output: the stitched panorama image
FloatImage stitchWarpLeft(const FloatImage &im1, const FloatImage &im2, const vector<vector<int>> im1Points, const vector<vector<int>> im2Points)
{
    if (im1Points.size() != im2Points.size() || im1Points.size() < 4) {
        throw MismatchedSizeException();
    }

    Matrix3f H = computeHomography(im1Points, im2Points);
    return stitch(im1, im2, H);
}

// Stitch 2 images together given 2 sets of corresponding points
// (this function is the same as stitchWarpLeft(), but warps both images instead of just the left)
// input: im1 and im2 are the left and right images to be stitched together,
// im1Points and im2Points are n-by-2 matrices holding the (x,y) locations
// of n point correspondences
// output: the stitched panorama image
FloatImage stitchWarpBoth(const FloatImage &im1, const FloatImage &im2, const vector<vector<int>> im1Points, const vector<vector<int>> im2Points)
{
    if (im1Points.size() != im2Points.size() || im1Points.size() < 4) {
        throw MismatchedSizeException();
    }

    // compute the average of the two sets of corresponding points
    vector<vector<int>> avgPoints;
    for (int i = 0; i < (int) im1Points.size(); i++) {
        vector<int> point;
        point.push_back((im1Points[i][0] + im2Points[i][0]) / 2);
        point.push_back((im1Points[i][1] + im2Points[i][1]) / 2);
        avgPoints.push_back(point);
    }
    // warp both images and determine output image size
    Matrix3f H1 = computeHomography(im1Points, avgPoints);
    Matrix3f H2 = computeHomography(im2Points, avgPoints);
    FloatImage outIm1 = warpImage(im1, H1);
    FloatImage outIm2 = warpImage(im2, H2);
    vector<float> bbox1 = computeTransformedBBox(im1.width(), im1.height(), H1);
    vector<float> bbox2 = computeTransformedBBox(im2.width(), im2.height(), H2);
    float tx1 = bbox1[0];
    float ty1 = bbox1[2];
    float tx2 = bbox2[0];
    int width = bbox2[1] - bbox1[0];
    int height = (int) (max(bbox1[3], bbox2[3]) - min(bbox1[2], bbox2[2]));

    // for sigmoid blending
    int overlap_x1 = bbox2[0] - tx1;
    int overlap_x2 = bbox1[1] - tx1;
    float mid = (overlap_x1 + overlap_x2) / 2.f;

    // combine warped images to form output image
    FloatImage output(width, height, im1.channels());
    for (int i = 0; i < output.width(); i++) {
		for (int j = 0; j < output.height(); j++) {
			for (int c = 0; c < output.channels(); c++) {
                float val1 = outIm1.smartAccessor(i, j, c, false);
                float val2 = outIm2.smartAccessor(i + tx1 - tx2, j + ty1, c, false);
				output(i, j, c) = val1 + val2;
                if (val1 != 0 && val2 != 0) {
                    // output(i, j, c) = val1; // simple overlap
                    // float alpha = ((float) i - overlap_x1) / (overlap_x2 - overlap_x1); // linear blend
                    float alpha = 1 / (1 + exp(mid - i)); // sigmoid blend
                    output(i, j, c) = val1 * (1 - alpha) + val2 * alpha;
                }
			}
		}
	}
    return output;
}
/* Part C 
Sources (some are re-listed next to specific functions)
* Mathematical explanation of cylindrical projection. This source was the basis for this section,and most of the theory was taken from this post: https://stackoverflow.com/questions/12017790/warp-image-to-appear-in-cylindrical-projection
* General background information: https://courses.cs.washington.edu/courses/cse576/08sp/lectures/Stitching.pdf
* General background information: https://graphics.stanford.edu/courses/cs448a-10/kari-panoramas-02mar10-opt.pdf
* General list of steps: http://pages.cs.wisc.edu/~vmathew/cs766-proj2/index.html
* Computing the focal length in mm: http://phototour.cs.washington.edu/focal.html
*/



// ------- PART C -------

// takes points from the original image and projects them onto the cylinder
// inputs: original x, original y, image width, image height, focal length, cylinder radius
// output: x,y coordinate after projection
// :information_source: https://stackoverflow.com/questions/12017790/warp-image-to-appear-in-cylindrical-projection this function is based off of
// this mathematical explanation and example. Everything was written ourselves, but the math behind it was taken from the post.
vector<float> convertToCylinder(float x, float y, int w, int h, float focal, float radius)
{
    //center the point at 0,0
    x = x - floor(w / 2);
    y = y - floor(h / 2);

    // calculate new coordinate
    float omega = w/2;
    float r2 = pow(radius, 2);
    float z0 = focal - sqrt(r2 - pow(omega, 2));
    float x2 = pow(x, 2);
    float f2 = pow(focal, 2);
    float z02 = pow(z0, 2);
    float zc = (2 * z0 + sqrt(4 * z02 - 4 * (x2 / f2 + 1) * (z02 - r2))) / (2 * (x2 / f2 + 1)); 
    float new_x = x * zc / focal;
    float new_y= y * zc / focal;

    // reconvert image coordinate
    new_x += floor(w / 2);
    new_y += floor(h / 2);

    vector<float> point;
    point.push_back(new_x);
    point.push_back(new_y);
    return point;
}

// takes an image and warps it to fit around a cylinder
// inputs: original image, focal length, radius
// outputs: warped image
FloatImage warpCylinder(const FloatImage &im, int focal, int radius){
    FloatImage result(im);
    for (int x = 0; x < im.width(); x++){
        for (int y = 0; y < im.height(); y++){
            for (int z = 0; z < im.depth(); z++){
                vector<float> new_coodinate = convertToCylinder(x, y, im.width(), im.height(), focal, radius);
                result(x, y, z) = interpolateLin(im, new_coodinate[0], new_coodinate[1], z, false);
            }
        }
    }   
    return result;
}

// takes a series of images and warps them all to a cylinder
// inputs: all the images, focal length, radius
// outputs: all the images, warped
vector<FloatImage> warpAll(vector<FloatImage> &images, int focal, int radius){
    vector<FloatImage> warped;
    for (FloatImage image : images){
        cout << "warping image" << endl;
        FloatImage result = warpCylinder(image, focal, radius);
        warped.push_back(result);
    }
    return warped;
}

// takes a series of images and the boundaries which to overlap the images at and creates a 360 panorama 
// inputs: original images, list of boundaries such that the first element is the left boundary of the first image,
// the second is the right boundary of the second image, the third is the left boundary of the second image, and so on
// and the focal length
// output: 360 panorama
FloatImage stitchCylinder(vector<FloatImage> &images, vector<int> boundaries, int focal){
    // warp images
    int circumference = calculateCircumference(boundaries);
    int radius = floor(circumference / (2 * M_PI));
    FloatImage result(circumference, circumference / 3, images[0].depth());
    vector<FloatImage> warped = warpAll(images, focal, radius);
    vector<int> newBoundaries = convertBoundaries(boundaries, radius, focal, images[0].width(), images[0].height());

    // keep track of what image we are on and what x coordinate we are on locally
    int currentImage = 0;
    int localX = newBoundaries[0];
    // add some black space to the top of the bottom in case the image is viewed in a 360 image viewer
    int offset = floor((circumference / 3 - images[0].height()) / 2);

    for (int x = 0; x < result.width(); x ++){
        bool blendBack = false;
        bool blendForward = false;
        // if we cross the boundary of the image we are on, move to the next one
        if (localX > newBoundaries[currentImage * 2 + 1]){
            currentImage += 1;
            localX = newBoundaries[currentImage * 2];
        }
        else{
            localX++;
        }
        // if near a seam, flag the pixel to be blended
        if (localX < newBoundaries[currentImage * 2] + 20 && currentImage >= 1){
            blendBack = true;
        }
        if (localX > newBoundaries[currentImage * 2 + 1] - 20 && currentImage <= (int) images.size() - 1){
            blendForward = true;
        }
        for (int y = offset; y < result.height() - offset; y ++){
            for (int z = 0; z < result.depth(); z ++){
                try
                {
                    // sigmoid blending
                    if (blendBack){
                        int diff = localX - newBoundaries[currentImage * 2];
                        float val1 = warped[currentImage](localX, y - offset, z);
                        float val2 = warped[currentImage - 1](newBoundaries[currentImage * 2 - 1] + diff, y - offset, z);
                        // result(x, y, z) = val1 + val2;
                        if (val1 != 0 && val2 != 0) {
                            float alpha = 1 / (1 + exp(diff)); 
                            result(x, y, z) = val1 * (1 - alpha) + val2 * alpha;
                        }
                    }
                    else if (blendForward){
                        int diff = newBoundaries[currentImage * 2 + 1] - localX;
                        float val1 = warped[currentImage](localX, y - offset, z);
                        float val2 = warped[currentImage + 1](newBoundaries[currentImage * 2 + 2] - diff, y - offset, z);
                        // result(x, y, z) = val1 + val2;
                        if (val1 != 0 && val2 != 0) {
                            float alpha = 1 / (1 + exp(diff)); 
                            result(x, y, z) = val1 * (1 - alpha) + val2 * alpha;
                        }
                    }
                    else{
                        //if no need to be blended copy the pixel value directly
                        result(x, y, z) = warped[currentImage](localX, y - offset, z);
                    }
   
                }
                catch(const std::exception& e)
                {
                    // sometimes the math doesnt work out exactly
                    std::cerr << e.what() << '\n';
                    std::cout << y << endl;
                }
                
                
            }
        }
    }
    return result;
}

// helper function that converts the boundaries to the cylindrical coordinates
// inputs: list of boundaries, radius, focal length, image width, image height
// outputs: converted boundaries
vector<int> convertBoundaries(vector<int> boundaries, int radius, int focal, int w, int h){
    vector<int> result;
    for (int boundary : boundaries){
        int new_x = floor(convertToCylinder(boundary, 0, w, h, focal, radius)[0]);
        result.push_back(new_x);
    }
    return result;
}

// helper function that calculates circumference of the cylinder
// inputs: list of boundaries
// output: circumference of the proposed cylinder
int calculateCircumference(const vector<int> boundaries){
    int circumference = 0;
    for (int i = 0; i <= (int) boundaries.size() - 1; i += 2 ){
        circumference += boundaries[i + 1] - boundaries[i];
    }
    cout<< "circumference = " << circumference << endl;
    return circumference;
}

// helper function that calculates focal length in pixels
// inputs: focal length in mm. sensor width, the image
// output: focal length in px
// :information_source: http://phototour.cs.washington.edu/focal.html equation was taken from this site 
int getFocalLength(float focalMM, float sensorWidth, FloatImage &im){
    return floor(im.width() * focalMM / sensorWidth);
}
