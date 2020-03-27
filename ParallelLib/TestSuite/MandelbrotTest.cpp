#include "stdafx.h"
#include <omp.h>
#include <cmath> //fabs
#include "MandelbrotTest.h"

#ifdef __GNUC__
#define OPENFILE(f, name, permissions) f = fopen(name, permissions)
#define WRITE fprintf
#else
#define OPENFILE(f, name, permissions) fopen_s(&f, name, permissions)
#define WRITE fprintf_s
#endif

// Note: Test core is based on program published on Rosetta Code:
// https://rosettacode.org/wiki/Mandelbrot_set#PPM_non_interactive

#define TEST_CORE(thread_id)                                                                     \
/*for (int iY = 0; iY < config.iYmax; iY++) <-paralleled for - to bedeclared outside macro */    \
{                                                                                                \
	Cy = testConfig.CyMin + iY * PixelHeight;                                                    \
	if (fabs(Cy) < PixelHeight / 2) Cy = 0.0;                                                    \
	                                                                                             \
	for (int iX = 0; iX < testConfig.iXmax; iX++)                                                \
	{                                                                                            \
		Cx = testConfig.CxMin + iX * PixelWidth;                                                 \
		/*initial value of orbit = critical point Z = 0*/                                        \
		Zx = 0.0;                                                                                \
		Zy = 0.0;                                                                                \
		Zx2 = Zx * Zx;                                                                           \
		Zy2 = Zy * Zy;                                                                           \
		                                                                                         \
		int Iteration = 0;                                                                       \
		for (Iteration; Iteration < testConfig.IterationMax && ((Zx2 + Zy2) < ER2); Iteration++) \
		{                                                                                        \
			Zy = 2 * Zx*Zy + Cy;                                                                 \
			Zx = Zx2 - Zy2 + Cx;                                                                 \
			Zx2 = Zx * Zx;                                                                       \
			Zy2 = Zy * Zy;                                                                       \
		};                                                                                       \
		/*compute  pixel color (24 bit = 3 bytes)*/                                              \
		if (Iteration == testConfig.IterationMax) /* interior */                                 \
		{                                                                                        \
			Image[((iY * testConfig.iYmax) + iX) * 3 + 0] = 0;                                   \
			Image[((iY * testConfig.iYmax) + iX) * 3 + 1] = 0;                                   \
			Image[((iY * testConfig.iYmax) + iX) * 3 + 2] = 0;                                   \
		}                                                                                        \
		else /* exterior */                                                                      \
		{                                                                                        \
			Image[((iY * testConfig.iYmax) + iX) * 3 + 0] = 255;                                 \
			Image[((iY * testConfig.iYmax) + iX) * 3 + 1] = 255;                                 \
			Image[((iY * testConfig.iYmax) + iX) * 3 + 2] = 255;                                 \
		};                                                                                       \
	}                                                                                            \
}

MandelbrotTest::MandelbrotTest(const std::string& inName, MandelbrotTestConfig& testConfig) 
	: Test(TestType::Mandelbrot, inName)
	, testConfig(testConfig)
{ }

MandelbrotTest::~MandelbrotTest() { }

std::string MandelbrotTest::BuildFilename(const TestParams& In)
{
	std::stringstream filenameBuilder;
	filenameBuilder << GetTestName();
	filenameBuilder << "-" << std::string(LibraryToString(GetRunningLibrary()));
	filenameBuilder << "-" << std::string(ForScheduleToString(In.forSchedule));
	filenameBuilder << "-thr" << In.numThreadsToUse;
	filenameBuilder << "-chunk" << In.forChunkSize;
	filenameBuilder << "_" << GetTestNum();
	filenameBuilder << ".ppm";
	return filenameBuilder.str();
}

void MandelbrotTest::DoSequentially(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();

	//image data
	unsigned char* Image = new unsigned char[testConfig.iYmax * testConfig.iXmax * 3];

	double PixelWidth = (testConfig.CxMax - testConfig.CxMin) / testConfig.iXmax;
	double PixelHeight = (testConfig.CyMax - testConfig.CyMin) / testConfig.iYmax;

	double Cx, Cy;
	double Zx, Zy;
	double Zx2, Zy2;
	//bail-out value , radius of circle ;  
	const double EscapeRadius = 2;
	double ER2 = EscapeRadius * EscapeRadius;

	FILE* fp = nullptr;
	if (testConfig.bExportImages)
	{
		//create new file,give it a name and open it in binary mode  
		std::string filename = BuildFilename(In);
		std::string comment = "# ";//comment should start with #
		OPENFILE(fp, filename.c_str(), "wb");
		WRITE(fp, "P6\n %s\n %d\n %d\n %d\n", comment.c_str(), testConfig.iXmax, testConfig.iYmax, testConfig.MaxColorComponentValue);
	}

	uint8_t* ImagePtr = Image;

	Out.BeginParallelWorkload();

	for (int iY = 0; iY < testConfig.iYmax; iY++)
	{
		TEST_CORE(0);
	}

	Out.BeginResourceCleanup();

	if (testConfig.bExportImages)
	{
		fwrite(Image, 1, testConfig.iXmax * testConfig.iYmax * 3, fp);
		fclose(fp);
		fp = nullptr;
	}
	delete Image;

	Out.EndTask(true);
}

#include "ParallelLib/ParallelLib.h"
void MandelbrotTest::DoParallelLib(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();

	//image data
	unsigned char* Image = new unsigned char[testConfig.iYmax * testConfig.iXmax * 3];

	double PixelWidth = (testConfig.CxMax - testConfig.CxMin) / testConfig.iXmax;
	double PixelHeight = (testConfig.CyMax - testConfig.CyMin) / testConfig.iYmax;

	//bail-out value , radius of circle ;  
	const double EscapeRadius = 2;
	double ER2 = EscapeRadius * EscapeRadius;

	//create new file,give it a name and open it in binary mode  
	FILE* fp = nullptr;
	if (testConfig.bExportImages)
	{
		std::string filename = BuildFilename(In);
		std::string comment = "# ";//comment should start with #
		OPENFILE(fp, filename.c_str(), "wb");
		WRITE(fp, "P6\n %s\n %d\n %d\n %d\n", comment.c_str(), testConfig.iXmax, testConfig.iYmax, testConfig.MaxColorComponentValue);
	}

	create_private(double, Cy);
	create_private(double, Cx);
	create_private(double, Zx);
	create_private(double, Zy);
	create_private(double, Zx2);
	create_private(double, Zy2);

	Out.BeginParallelWorkload();

	//      pFor yLoop executes for(int iY = 0; i < config.iYmax; iY += 1)
	parallel_for(yLoop,                 iY , 0,     testConfig.iYmax,       1, num_threads(In.numThreadsToUse) schedulev((pSchedule)In.forSchedule, In.forChunkSize),
	{
		TEST_CORE(0);
	});
	
	Out.BeginResourceCleanup();

	if (testConfig.bExportImages)
	{
		fwrite(Image, 1, testConfig.iXmax * testConfig.iYmax * 3, fp);
		fclose(fp);
		fp = nullptr;
	}
	delete Image;

	Out.EndTask(true);
}
#undef parallel_for

void MandelbrotTest::DoOpenMP(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();

	//image data
	unsigned char* Image = new unsigned char[testConfig.iYmax * testConfig.iXmax * 3];

	double PixelWidth = (testConfig.CxMax - testConfig.CxMin) / testConfig.iXmax;
	double PixelHeight = (testConfig.CyMax - testConfig.CyMin) / testConfig.iYmax;

	double Cx, Cy;
	double Zx, Zy;
	double Zx2, Zy2;

	//bail-out value , radius of circle ;
	double ER2 = testConfig.EscapeRadius * testConfig.EscapeRadius;

	//create new file,give it a name and open it in binary mode  
	FILE* fp = nullptr;
	if (testConfig.bExportImages)
	{
		std::string filename = BuildFilename(In);
		std::string comment = "# ";//comment should start with #
		OPENFILE(fp, filename.c_str(), "wb");
		WRITE(fp, "P6\n %s\n %d\n %d\n %d\n", comment.c_str(), testConfig.iXmax, testConfig.iYmax, testConfig.MaxColorComponentValue);
	}

	omp_set_num_threads(In.numThreadsToUse);
	Out.BeginParallelWorkload();

	#undef schedule
	#undef num_threads
	switch (In.forSchedule)
	{
	case ForSchedule::Static:
		#pragma omp parallel for private(Cy,Cx, Zx, Zy, Zx2, Zy2) schedule(static, In.forChunkSize) num_threads(In.numThreadsToUse)
		for (int iY = 0; iY < testConfig.iYmax; iY++)
		{ TEST_CORE(0); }
		break;

	case ForSchedule::Dynamic:
		#pragma omp parallel for private(Cy,Cx, Zx, Zy, Zx2, Zy2) schedule(dynamic, In.forChunkSize) num_threads(In.numThreadsToUse)
		for (int iY = 0; iY < testConfig.iYmax; iY++)
		{
			TEST_CORE(0);
		}
		break;

	case ForSchedule::Guided:
		#pragma omp parallel for private(Cy,Cx, Zx, Zy, Zx2, Zy2) schedule(guided, In.forChunkSize) num_threads(In.numThreadsToUse)
		for (int iY = 0; iY < testConfig.iYmax; iY++)
		{
			TEST_CORE(0);
		}
		break;
	}
	
	Out.BeginResourceCleanup();
	
	if (testConfig.bExportImages)
	{
		fwrite(Image, 1, testConfig.iXmax * testConfig.iYmax * 3, fp);
		fclose(fp);
		fp = nullptr;
	}
	delete Image;

	Out.EndTask(true);
}

#define TBB_USE_EXCEPTIONS 0
#include "tbb/parallel_for.h"
void MandelbrotTest::DoTBB(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();

	//image data
	unsigned char* Image = new unsigned char[testConfig.iYmax * testConfig.iXmax * 3];

	double PixelWidth = (testConfig.CxMax - testConfig.CxMin) / testConfig.iXmax;
	double PixelHeight = (testConfig.CyMax - testConfig.CyMin) / testConfig.iYmax;

	//bail-out value , radius of circle ;  
	const double EscapeRadius = 2;
	double ER2 = EscapeRadius * EscapeRadius;
	
	//create new file,give it a name and open it in binary mode  
	FILE* fp = nullptr;
	if (testConfig.bExportImages)
	{
		std::string filename = BuildFilename(In);
		std::string comment = "# ";//comment should start with #
		OPENFILE(fp, filename.c_str(), "wb");
		WRITE(fp, "P6\n %s\n %d\n %d\n %d\n", comment.c_str(), testConfig.iXmax, testConfig.iYmax, testConfig.MaxColorComponentValue);
	}

	thread_local double Cy;
	thread_local double Cx;
	thread_local double Zx;
	thread_local double Zy;
	thread_local double Zx2;
	thread_local double Zy2;

	Out.BeginParallelWorkload();

	//      pFor yLoop executes for(int iY = 0; i < config.iYmax; iY += 1)
	//parallel_for(yLoop, iY, 0, config.iYmax, 1, num_threads(In.numThreadsToUse) schedulev((pSchedule)In.forSchedule, In.forChunkSize),

	switch (In.forSchedule)
	{
	case ForSchedule::Static:
		tbb::parallel_for(0, testConfig.iYmax, [&](size_t iY)
		{
			TEST_CORE(0);
		}, tbb::static_partitioner());
		break;

	case ForSchedule::Dynamic:
		tbb::parallel_for(0, testConfig.iYmax, [&](size_t iY)
		{
			TEST_CORE(0);
		});
		break;

	case ForSchedule::Guided:
		throw; //not available
	}

	Out.BeginResourceCleanup();

	if (testConfig.bExportImages)
	{
		fwrite(Image, 1, testConfig.iXmax * testConfig.iYmax * 3, fp);
		fclose(fp);
		fp = nullptr;
	}
	delete Image;

	Out.EndTask(true);
}

#undef TimeStamp
#include "dlib/threads/parallel_for_extension.h"
void MandelbrotTest::DoDlib(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();

	//image data
	unsigned char* Image = new unsigned char[testConfig.iYmax * testConfig.iXmax * 3];

	double PixelWidth = (testConfig.CxMax - testConfig.CxMin) / testConfig.iXmax;
	double PixelHeight = (testConfig.CyMax - testConfig.CyMin) / testConfig.iYmax;

	//bail-out value , radius of circle ;  
	const double EscapeRadius = 2;
	double ER2 = EscapeRadius * EscapeRadius;
	
	//create new file,give it a name and open it in binary mode  
	FILE* fp = nullptr;
	if (testConfig.bExportImages)
	{
		std::string filename = BuildFilename(In);
		std::string comment = "# ";//comment should start with #
		OPENFILE(fp, filename.c_str(), "wb");
		WRITE(fp, "P6\n %s\n %d\n %d\n %d\n", comment.c_str(), testConfig.iXmax, testConfig.iYmax, testConfig.MaxColorComponentValue);
	}

	thread_local double Cy;
	thread_local double Cx;
	thread_local double Zx;
	thread_local double Zy;
	thread_local double Zx2;
	thread_local double Zy2;

	dlib::thread_pool dpool(In.numThreadsToUse);

	Out.BeginParallelWorkload();


	switch (In.forSchedule)
	{
	case ForSchedule::Static:
		throw; //not available
		break;

	case ForSchedule::Dynamic:
		dlib::parallel_for(dpool, 0, testConfig.iYmax, [&](long iY) { TEST_CORE(0); }, In.forChunkSize);
		break;

	case ForSchedule::Guided:
		throw; //not available
	}

	Out.BeginResourceCleanup();

	if (testConfig.bExportImages)
	{
		fwrite(Image, 1, testConfig.iXmax * testConfig.iYmax * 3, fp);
		fclose(fp);
		fp = nullptr;
	}
	delete Image;

	Out.EndTask(true);
}
