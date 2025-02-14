// st_detail_try2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <fstream>
#include <string>
#include <iostream>
#include <fstream> 
#include <string>
#include <iomanip> 
#include "opencv2/opencv_modules.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/stitching/detail/autocalib.hpp"
#include "opencv2/stitching/detail/blenders.hpp"
#include "opencv2/stitching/detail/camera.hpp"
#include "opencv2/stitching/detail/exposure_compensate.hpp"
#include "opencv2/stitching/detail/matchers.hpp"
#include "opencv2/stitching/detail/motion_estimators.hpp"
#include "opencv2/stitching/detail/seam_finders.hpp"
#include "opencv2/stitching/detail/util.hpp"
#include "opencv2/stitching/detail/warpers.hpp"
#include "opencv2/stitching/warpers.hpp"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include <opencv2/core/types_c.h>

using namespace std;
using namespace cv;
using namespace cv::detail;


//
#define ENABLE_LOG 0

// Default command line args
vector<string> img_names;
bool preview = false;
bool try_gpu = true;
double work_megapix = 0.6;
double seam_megapix = 0.1;
double compose_megapix = -1;
float conf_thresh = 1.f;
string features_type = "surf";
string ba_cost_func = "ray";
string ba_refine_mask = "xxxxx";
bool do_wave_correct = true;
WaveCorrectKind wave_correct = detail::WAVE_CORRECT_HORIZ;
bool save_graph = false;
std::string save_graph_to;
string warp_type = "cylindrical";
//string warp_type = "cylindrical";
int expos_comp_type = ExposureCompensator::GAIN_BLOCKS;
float match_conf = 0.3f;
//string seam_find_type = "gc_color";
string seam_find_type = "gc_color";
int blend_type = Blender::MULTI_BAND;
float blend_strength = 3;
string result_name = "result.jpg";

//Mat result, result_mask;

int main(int argc, char* argv[])
{
	//读入图像
	VideoCapture capleft(0);
	VideoCapture capright(1);

	double rate = 60;
	int delay = 1000 / rate;
	bool stop(false);
	Mat frame1;
	Mat frame2;
	Mat frame;
	int k = 20;

	Mat frameCalibration;
	Size imageSize;
	string imgname;
	string calimgname;
	string calimgname1;
	Mat view, rview, map1left, map2left;
	Mat map1right, map2right;

	capleft.set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));
	capleft.set(CV_CAP_PROP_FPS, 20);
	capleft.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
	capleft.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);

	capright.set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));
	capright.set(CV_CAP_PROP_FPS, 20);
	capright.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
	capright.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
	capleft.set(CV_CAP_PROP_FOCUS, 0);
	capright.set(CV_CAP_PROP_FOCUS, 0);

	capleft.read(frame1);

	Mat cameraMatrixLeft = Mat::eye(3, 3, CV_64F);
	cameraMatrixLeft.at<double>(0, 0) = 883.084143978625;
	cameraMatrixLeft.at<double>(0, 1) = -1.47411150609196;
	cameraMatrixLeft.at<double>(0, 2) = 959.966704458933;
	cameraMatrixLeft.at<double>(1, 1) = 891.885889942979;
	cameraMatrixLeft.at<double>(1, 2) = 510.221512147748;

	Mat distCoeffsLeft = Mat::zeros(5, 1, CV_64F);
	distCoeffsLeft.at<double>(0, 0) = -0.241829069919463;
	distCoeffsLeft.at<double>(1, 0) = 0.0423113637454452;
	distCoeffsLeft.at<double>(2, 0) = -0.000888053247768027;
	distCoeffsLeft.at<double>(3, 0) = -0.000219180509369160;
	distCoeffsLeft.at<double>(4, 0) = 0;

	Mat cameraMatrixRight = Mat::eye(3, 3, CV_64F);
	cameraMatrixRight.at<double>(0, 0) = 883.084143978625;
	cameraMatrixRight.at<double>(0, 1) = -1.47411150609196;
	cameraMatrixRight.at<double>(0, 2) = 959.966704458933;
	cameraMatrixRight.at<double>(1, 1) = 691.885889942979;
	cameraMatrixRight.at<double>(1, 2) = 510.221512147748;

	Mat distCoeffsRight = Mat::zeros(5, 1, CV_64F);
	distCoeffsRight.at<double>(0, 0) = -0.241829069919463;
	distCoeffsRight.at<double>(1, 0) = 0.0423113637454452;
	distCoeffsRight.at<double>(2, 0) = -0.000888053247768027;
	distCoeffsRight.at<double>(3, 0) = -0.000219180509369160;
	distCoeffsRight.at<double>(4, 0) = 0;

	imageSize = frame1.size();
	initUndistortRectifyMap(cameraMatrixLeft, distCoeffsLeft, Mat(),
		getOptimalNewCameraMatrix(cameraMatrixLeft, distCoeffsLeft, imageSize, 1, imageSize, 0),
		imageSize, CV_16SC2, map1left, map2left);
	initUndistortRectifyMap(cameraMatrixRight, distCoeffsRight, Mat(),
		getOptimalNewCameraMatrix(cameraMatrixLeft, distCoeffsLeft, imageSize, 1, imageSize, 0),
		imageSize, CV_16SC2, map1right, map2right);
	/*
	while (k--)
	{
		if (capleft.read(frame1) && capright.read(frame2))
		{
			imshow("cam1", frame1);
			imshow("cam2", frame2);
			resize(frame1, frame1, Size(480, 290));
			resize(frame2, frame2, Size(480, 290));
			imwrite("camop2.jpg", frame1);
			imwrite("camop1.jpg", frame2);
		}
	}*/
	//img_names.push_back("rockfeller_1.jpg");
	//img_names.push_back("rockfeller_2.jpg");
	//img_names.push_back("usedcal1.jpg");
	//img_names.push_back("usedcal2.jpg");
	img_names.push_back("camop2.jpg");
	img_names.push_back("camop1.jpg");

#if ENABLE_LOG
	int64 app_start_time = getTickCount();
#endif

	cv::setBreakOnError(true);

	/*int retval = parseCmdArgs(argc, argv);
	if (retval)
		return retval;*/

		// Check if have enough images
	int num_images = static_cast<int>(img_names.size());
	if (num_images < 2)
	{
		//LOGLN("Need more images");
		return -1;
	}

	double work_scale = 1, seam_scale = 1, compose_scale = 1;
	bool is_work_scale_set = false, is_seam_scale_set = false, is_compose_scale_set = false;
	bool is_registration_finished = false;
	//LOGLN("Finding features...");
#if ENABLE_LOG
	int64 t = getTickCount();
#endif

	Ptr<FeaturesFinder> finder;
	//cut 1
	finder = new SurfFeaturesFinder();


	Mat full_img, img;
	vector<ImageFeatures> features(num_images);
	vector<Mat> images(num_images);
	vector<Size> full_img_sizes(num_images);
	double seam_work_aspect = 1;



	for (int i = 0; i < num_images; ++i)
	{
		full_img = imread(img_names[i]);
		full_img_sizes[i] = full_img.size();

		if (full_img.empty())
		{
			//LOGLN("Can't open image " << img_names[i]);
			return -1;
		}
		if (work_megapix < 0)
		{
			img = full_img;
			work_scale = 1;
			is_work_scale_set = true;
		}
		else
		{
			if (!is_work_scale_set)
			{
				work_scale = min(1.0, sqrt(work_megapix * 1e6 / full_img.size().area()));
				is_work_scale_set = true;
			}
			resize(full_img, img, Size(), work_scale, work_scale);
		}
		if (!is_seam_scale_set)
		{
			seam_scale = min(1.0, sqrt(seam_megapix * 1e6 / full_img.size().area()));
			seam_work_aspect = seam_scale / work_scale;
			is_seam_scale_set = true;
		}

		(*finder)(img, features[i]);
		features[i].img_idx = i;
		
		resize(full_img, img, Size(), seam_scale, seam_scale);
		images[i] = img.clone();
	}

	finder->collectGarbage();
	full_img.release();
	img.release();

	//LOGLN("Finding features, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec");

	//LOG("Pairwise matching");
#if ENABLE_LOG
	t = getTickCount();
#endif
	vector<MatchesInfo> pairwise_matches;
	BestOf2NearestMatcher matcher(try_gpu, match_conf);
	matcher(features, pairwise_matches);
	matcher.collectGarbage();
	//LOGLN("Pairwise matching, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec");

	// Check if we should save matches graph
	//if (save_graph)//no
	//{
		//LOGLN("Saving matches graph...");
	//	ofstream f(save_graph_to.c_str());
	//	f << matchesGraphAsString(img_names, pairwise_matches, conf_thresh);
	//}

	// Leave only images we are sure are from the same panorama
	vector<int> indices = leaveBiggestComponent(features, pairwise_matches, conf_thresh);
	vector<Mat> img_subset;
	vector<string> img_names_subset;
	vector<Size> full_img_sizes_subset;
	for (size_t i = 0; i < indices.size(); ++i)
	{
		img_names_subset.push_back(img_names[indices[i]]);
		img_subset.push_back(images[indices[i]]);
		full_img_sizes_subset.push_back(full_img_sizes[indices[i]]);
	}

	images = img_subset;
	img_names = img_names_subset;
	full_img_sizes = full_img_sizes_subset;

	// Check if we still have enough images
	num_images = static_cast<int>(img_names.size());
	if (num_images < 2)
	{
		//LOGLN("Need more images");
		return -1;
	}

	//double ttt = getTickCount();
	//以下需要20sec

	HomographyBasedEstimator estimator;
	vector<CameraParams> cameras;
	estimator(features, pairwise_matches, cameras);
	//features表示所有待拼接图像的特征
	//pairwise_matches表示匹配点对
	//cameras表示相机参数信息

	//same to estimateFocal
	for (size_t i = 1; i < 3; i++)
	{
		const double* h = reinterpret_cast<const double*>(pairwise_matches[i].H.data);
		cout << "单应矩阵：" << endl;
		cout << setw(10) << (int)(h[0] + 0.5) << setw(6) << (int)(h[1] + 0.5) << setw(6) << (int)(h[2] + 0.5) << endl;
		cout << setw(10) << (int)(h[3] + 0.5) << setw(6) << (int)(h[4] + 0.5) << setw(6) << (int)(h[5] + 0.5) << endl;
		cout << setw(10) << (int)(h[6] + 0.5) << setw(6) << (int)(h[7] + 0.5) << setw(6) << (int)(h[8] + 0.5) << endl;
	}

	for (size_t i = 0; i < cameras.size(); ++i)
	{
		Mat R;
		cameras[i].R.convertTo(R, CV_32F);
		cameras[i].R = R;

		//LOGLN("Initial intrinsics #" << indices[i] + 1 << ":\n" << cameras[i].K());
	}

	Ptr<detail::BundleAdjusterBase> adjuster;
	if (ba_cost_func == "reproj") adjuster = new detail::BundleAdjusterReproj();
	else if (ba_cost_func == "ray") adjuster = new detail::BundleAdjusterRay();
	else
	{
		//cout << "Unknown bundle adjustment cost function: '" << ba_cost_func << "'.\n";
		return -1;
	}
	adjuster->setConfThresh(conf_thresh);
	Mat_<uchar> refine_mask = Mat::zeros(3, 3, CV_8U);
	if (ba_refine_mask[0] == 'x') refine_mask(0, 0) = 1;
	if (ba_refine_mask[1] == 'x') refine_mask(0, 1) = 1;
	if (ba_refine_mask[2] == 'x') refine_mask(0, 2) = 1;
	if (ba_refine_mask[3] == 'x') refine_mask(1, 1) = 1;
	if (ba_refine_mask[4] == 'x') refine_mask(1, 2) = 1;
	adjuster->setRefinementMask(refine_mask);
	(*adjuster)(features, pairwise_matches, cameras);
	//BundleAdjusterRay

	// Find median focal length
	vector<double> focals;
	for (size_t i = 0; i < cameras.size(); ++i)
	{
		//LOGLN("Camera #" << indices[i] + 1 << ":\n" << cameras[i].K());
		focals.push_back(cameras[i].focal);
	}

	sort(focals.begin(), focals.end());
	float warped_image_scale;
	if (focals.size() % 2 == 1)
		warped_image_scale = static_cast<float>(focals[focals.size() / 2]);
	else
		warped_image_scale = static_cast<float>(focals[focals.size() / 2 - 1] + focals[focals.size() / 2]) * 0.5f;

	if (do_wave_correct)
	{
		vector<Mat> rmats;
		for (size_t i = 0; i < cameras.size(); ++i)
			rmats.push_back(cameras[i].R.clone());
		waveCorrect(rmats, wave_correct);
		for (size_t i = 0; i < cameras.size(); ++i)
			cameras[i].R = rmats[i];
	}

	//LOGLN("Warping images (auxiliary)... ");
#if ENABLE_LOG
	t = getTickCount();
#endif

	vector<Point> corners(num_images);
	vector<Mat> masks_warped(num_images);
	vector<Mat> images_warped(num_images);
	vector<Size> sizes(num_images);
	vector<Mat> masks(num_images);

	// Preapre images masks
	for (int i = 0; i < num_images; ++i)
	{
		masks[i].create(images[i].size(), CV_8U);
		masks[i].setTo(Scalar::all(255));
	}

	// Warp images and their masks

	Ptr<WarperCreator> warper_creator;


	if (warp_type == "plane") warper_creator = new cv::PlaneWarper();
	else if (warp_type == "cylindrical") warper_creator = new cv::CylindricalWarper();
	else if (warp_type == "spherical") warper_creator = new cv::SphericalWarper();
	else if (warp_type == "fisheye") warper_creator = new cv::FisheyeWarper();
	else if (warp_type == "stereographic") warper_creator = new cv::StereographicWarper();
	else if (warp_type == "compressedPlaneA2B1") warper_creator = new cv::CompressedRectilinearWarper(2, 1);
	else if (warp_type == "compressedPlaneA1.5B1") warper_creator = new cv::CompressedRectilinearWarper(1.5, 1);
	else if (warp_type == "compressedPlanePortraitA2B1") warper_creator = new cv::CompressedRectilinearPortraitWarper(2, 1);
	else if (warp_type == "compressedPlanePortraitA1.5B1") warper_creator = new cv::CompressedRectilinearPortraitWarper(1.5, 1);
	else if (warp_type == "paniniA2B1") warper_creator = new cv::PaniniWarper(2, 1);
	else if (warp_type == "paniniA1.5B1") warper_creator = new cv::PaniniWarper(1.5, 1);
	else if (warp_type == "paniniPortraitA2B1") warper_creator = new cv::PaniniPortraitWarper(2, 1);
	else if (warp_type == "paniniPortraitA1.5B1") warper_creator = new cv::PaniniPortraitWarper(1.5, 1);
	else if (warp_type == "mercator") warper_creator = new cv::MercatorWarper();
	else if (warp_type == "transverseMercator") warper_creator = new cv::TransverseMercatorWarper();




	Ptr<RotationWarper> warper = warper_creator->create(static_cast<float>(warped_image_scale * seam_work_aspect));

	////
	for (int i = 0; i < num_images; ++i)
	{
		Mat_<float> K;
		cameras[i].K().convertTo(K, CV_32F);
		float swa = (float)seam_work_aspect;
		K(0, 0) *= swa; K(0, 2) *= swa;
		K(1, 1) *= swa; K(1, 2) *= swa;

		corners[i] = warper->warp(images[i], K, cameras[i].R, INTER_LINEAR, BORDER_REFLECT, images_warped[i]);
		sizes[i] = images_warped[i].size();

		warper->warp(masks[i], K, cameras[i].R, INTER_NEAREST, BORDER_CONSTANT, masks_warped[i]);

	}
	imshow("images_warped1", images_warped[0]);
	imshow("images_warped2", images_warped[1]);
	imshow("masks_warped1", images_warped[0]);
	imshow("masks_warped2", images_warped[1]);

	vector<Mat> images_warped_f(num_images);
	for (int i = 0; i < num_images; ++i)
	{
		images_warped[i].convertTo(images_warped_f[i], CV_32F);

	}
	//imshow("12", images_warped_f[0]);
	//imshow("21", images_warped_f[1]);
	//LOGLN("Warping images, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec");

	//开始曝光补偿
	Ptr<ExposureCompensator> compensator = ExposureCompensator::createDefault(expos_comp_type);
	compensator->feed(corners, images_warped, masks_warped);

	Ptr<SeamFinder> seam_finder;
	if (seam_find_type == "no")
		seam_finder = new detail::NoSeamFinder();
	else if (seam_find_type == "voronoi")
		seam_finder = new detail::VoronoiSeamFinder();
	else if (seam_find_type == "gc_color")
		seam_finder = new detail::GraphCutSeamFinder(GraphCutSeamFinderBase::COST_COLOR);
	else if (seam_find_type == "gc_colorgrad")
		seam_finder = new detail::GraphCutSeamFinder(GraphCutSeamFinderBase::COST_COLOR_GRAD);
	else if (seam_find_type == "dp_color")
		seam_finder = new detail::DpSeamFinder(DpSeamFinder::COLOR);
	else if (seam_find_type == "dp_colorgrad")
		seam_finder = new detail::DpSeamFinder(DpSeamFinder::COLOR_GRAD);
	seam_finder->find(images_warped_f, corners, masks_warped);

	// Release unused memory
	images.clear();
	images_warped.clear();
	images_warped_f.clear();
	masks.clear();

	//LOGLN("Compositing...");
#if ENABLE_LOG
	t = getTickCount();
#endif

	Mat Proto_frame1, Proto_frame2;
	//Proto_frame1 = imread("usedcal1.jpg");
	//Proto_frame2 = imread("usedcal2.jpg");
	Proto_frame1 = imread("camop2.jpg");
	Proto_frame2 = imread("camop1.jpg");
	while (1)
	{
		double ttt = getTickCount();
		double ttemp = getTickCount();
		Ptr<Blender> blender;
		Mat img_warped, img_warped_s;
		Mat dilated_mask, seam_mask, mask, mask_warped;
		//double compose_seam_aspect = 1;
		double compose_work_aspect = 1;
		if (capleft.read(frame1) && capright.read(frame2))
		{
			//cout << "gogogog" << endl;

			//remap(frame1, frame1, map1left, map2left, INTER_LINEAR);
			//remap(frame2, frame2, map1right, map2right, INTER_LINEAR);

			resize(frame1, frame1, Size(480, 290));
			resize(frame2, frame2, Size(480, 290));
			namedWindow("camera1", WINDOW_NORMAL);
			namedWindow("camera2", WINDOW_NORMAL);
			imshow("camera1", frame1);
			imshow("camera2", frame2);
			//double ttt = getTickCount();
			for (int img_idx = 0; img_idx < num_images; ++img_idx)
			{
				//LOGLN("Compositing image #" << indices[img_idx] + 1);

				// Read image and resize it if necessary
				if (!is_registration_finished)
				{
					switch (img_idx)
					{
					case 0:
						full_img = Proto_frame1;
						break;
					case 1:
						full_img = Proto_frame2;
						is_registration_finished = true;
						break;
					}
				}
				else
				{
					switch (img_idx)
					{
					case 0:
						full_img = frame1;
						break;
					case 1:
						full_img = frame2;
						break;
					}
				}
				//full_img = imread(img_names[img_idx]);
				if (!is_compose_scale_set)
				{

					if (compose_megapix > 0)
						compose_scale = min(1.0, sqrt(compose_megapix * 1e6 / full_img.size().area()));
					is_compose_scale_set = true;

					// Compute relative scales
					//compose_seam_aspect = compose_scale / seam_scale;
					compose_work_aspect = compose_scale / work_scale;
					//cout << compose_work_aspect << endl;
					// Update warped image scale
					warped_image_scale *= static_cast<float>(compose_work_aspect);
					warper = warper_creator->create(warped_image_scale);

					// Update corners and sizes
					for (int i = 0; i < num_images; ++i)
					{
						// Update intrinsics
						cameras[i].focal *= compose_work_aspect;
						cameras[i].ppx *= compose_work_aspect;
						cameras[i].ppy *= compose_work_aspect;

						// Update corner and size
						Size sz = full_img_sizes[i];
						if (std::abs(compose_scale - 1) > 1e-1)
						{
							sz.width = cvRound(full_img_sizes[i].width * compose_scale);
							sz.height = cvRound(full_img_sizes[i].height * compose_scale);
						}

						Mat K;
						cameras[i].K().convertTo(K, CV_32F);
						Rect roi = warper->warpRoi(sz, K, cameras[i].R);
						corners[i] = roi.tl();
						sizes[i] = roi.size();
					}
				}
				if (abs(compose_scale - 1) > 1e-1) {
					resize(full_img, img, Size(), compose_scale, compose_scale);
					cout << "compose_scale" << endl;
					//didn't get in this column
				}
				else
					img = full_img;
				full_img.release();
				Size img_size = img.size();

				Mat K;
				cameras[img_idx].K().convertTo(K, CV_32F);

				//ttemp = getTickCount();
				// Warp the current image
				ttemp = getTickCount();

				warper->warp(img, K, cameras[img_idx].R, INTER_LINEAR, BORDER_REFLECT, img_warped);

				// Warp the current image mask
				mask.create(img_size, CV_8U);
				mask.setTo(Scalar::all(255));

				cout << "warper_img:" << ttemp/10000000000 << endl;
				ttemp = getTickCount();
				warper->warp(mask, K, cameras[img_idx].R, INTER_NEAREST, BORDER_CONSTANT, mask_warped);
				imshow("gogo", mask_warped);
				// Compensate exposure
				//compensator->apply(img_idx, corners[img_idx], img_warped, mask_warped);

				img_warped.convertTo(img_warped_s, CV_16S);
				img_warped.release();
				img.release();
				mask.release();

				ttemp = ((double)getTickCount() - ttemp) / getTickFrequency();
				cout << "warper_img_mask:" << ttemp << endl;

				dilate(masks_warped[img_idx], dilated_mask, Mat());
				resize(dilated_mask, seam_mask, mask_warped.size());
				imshow("seam_mask",seam_mask);
				mask_warped = seam_mask & mask_warped;

				ttemp = getTickCount();
				if (blender.empty())
				{
					blender = Blender::createDefault(blend_type, try_gpu);
					Size dst_sz = resultRoi(corners, sizes).size();
					float blend_width = sqrt(static_cast<float>(dst_sz.area())) * blend_strength / 100.f;
					if (blend_width < 1.f)
						blender = Blender::createDefault(Blender::NO, try_gpu);
					else if (blend_type == Blender::MULTI_BAND)
					{
						MultiBandBlender* mb = dynamic_cast<MultiBandBlender*>(static_cast<Blender*>(blender));
						mb->setNumBands(static_cast<int>(ceil(log(blend_width) / log(2.)) - 1.));
						//LOGLN("Multi-band blender, number of bands: " << mb->numBands());
					}
					else if (blend_type == Blender::FEATHER)
					{
						FeatherBlender* fb = dynamic_cast<FeatherBlender*>(static_cast<Blender*>(blender));
						fb->setSharpness(1.f / blend_width);
						//LOGLN("Feather blender, sharpness: " << fb->sharpness());
					}
					blender->prepare(corners, sizes);
				}

				// Blend the current image
				blender->feed(img_warped_s, mask_warped, corners[img_idx]);
				ttemp = ((double)getTickCount() - ttemp) / getTickFrequency();
				cout << "blending:" << ttemp << endl;
			}

			Mat result, result_mask;
			blender->blend(result, result_mask);


			result.convertTo(result, CV_8UC1);
			namedWindow("stitch", WINDOW_NORMAL);
			imshow("stitch", result);
			ttt = ((double)getTickCount() - ttt) / getTickFrequency();
			cout << "总的拼接时间:" << ttt << endl;
			//waitKey(2000);
			if (char key = waitKey(1))//按ESC键
			{
				if (key == 'p')
				{
					//imwrite("cam1.jpg", frame2);
					//imwrite("cam2.jpg", frame1);
					stop = true;
					cout << "程序结束！" << endl;
					cout << "*** ***" << endl;
				}
			}
		}

	}

	waitKey(0);

	//LOGLN("Finished, total time: " << ((getTickCount() - app_start_time) / getTickFrequency()) << " sec");
	return 0;
}