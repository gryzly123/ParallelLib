#include "stdafx.h"
#include "ParallelLib.h"
#include <omp.h>
#include "MandelbrotTest.h"

MandelbrotTest::MandelbrotTest(const std::string& inName) : Test(TestType::Mandelbrot, inName) { }

MandelbrotTest::~MandelbrotTest() { }

void MandelbrotTest::DoSequentially(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();

	//image data
	unsigned char* Image = new unsigned char[config.iYmax * config.iXmax * 3];

	double PixelWidth = (config.CxMax - config.CxMin) / config.iXmax;
	double PixelHeight = (config.CyMax - config.CyMin) / config.iYmax;

	FILE* fp;
	std::string filename = "seq1.ppm";
	std::string comment = "# ";//comment should start with #

	double Cx, Cy;
	double Zx, Zy;
	double Zx2, Zy2;

	//bail-out value , radius of circle ;  
	const double EscapeRadius = 2;
	double ER2 = EscapeRadius * EscapeRadius;

	//create new file,give it a name and open it in binary mode  
	fopen_s(&fp, filename.c_str(), "wb");
	fprintf_s(fp, "P6\n %s\n %d\n %d\n %d\n", comment.c_str(), config.iXmax, config.iYmax, config.MaxColorComponentValue);

	uint8_t* ImagePtr = Image;
	++ImagePtr;

	Out.BeginParallelWorkload();

	for (int iY = 0; iY < config.iYmax; iY++)
	{
		Cy = config.CyMin + iY * PixelHeight;
		if (fabs(Cy) < PixelHeight / 2) Cy = 0.0;

		for (int iX = 0; iX < config.iXmax; iX++)
		{
			Cx = config.CxMin + iX * PixelWidth;
			//initial value of orbit = critical point Z= 0 
			Zx = 0.0;
			Zy = 0.0;
			Zx2 = Zx * Zx;
			Zy2 = Zy * Zy;

			int Iteration = 0;
			for (Iteration; Iteration < config.IterationMax && ((Zx2 + Zy2) < ER2); Iteration++)
			{
				Zy = 2 * Zx*Zy + Cy;
				Zx = Zx2 - Zy2 + Cx;
				Zx2 = Zx * Zx;
				Zy2 = Zy * Zy;
			};
			//compute  pixel color (24 bit = 3 bytes) 
			if (Iteration == config.IterationMax)
			{ // interior of Mandelbrot set = black 
				*(ImagePtr++) = 0;
				*(ImagePtr++) = 0;
				*(ImagePtr++) = 0;
			}
			else
			{ //exterior of Mandelbrot set = white 
				*(ImagePtr++) = 255; //Red
				*(ImagePtr++) = 255; //Green 
				*(ImagePtr++) = 255;//Blue 
			};
			//if (Iteration == config.IterationMax)
			//{ // interior of Mandelbrot set = black 
			//	Image[(iY * config.iXmax) + iX + 0] = 0;
			//	Image[(iY * config.iXmax) + iX + 1] = 0;
			//	Image[(iY * config.iXmax) + iX + 2] = 0;
			//}
			//else
			//{ //exterior of Mandelbrot set = white 
			//	Image[(iY * config.iXmax) + iX + 0] = 255; //Red
			//	Image[(iY * config.iXmax) + iX + 1] = 255; //Green 
			//	Image[(iY * config.iXmax) + iX + 2] = 255;//Blue 
			//};
		}
	}

	Out.BeginResourceCleanup();

	fwrite(Image, 1, config.iXmax * config.iYmax * 3, fp);
	fclose(fp);
	delete Image;

	Out.EndTask(true);
}

void MandelbrotTest::DoParallelLib(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();

	//image data
	unsigned char* Image = new unsigned char[config.iYmax * config.iXmax * 3];

	double PixelWidth = (config.CxMax - config.CxMin) / config.iXmax;
	double PixelHeight = (config.CyMax - config.CyMin) / config.iYmax;

	FILE* fp;
	std::string filename = "palA.ppm";
	std::string comment = "# ";//comment should start with #

	double Cx;
	double Zx, Zy;
	double Zx2, Zy2;

	//bail-out value , radius of circle ;  
	const double EscapeRadius = 2;
	double ER2 = EscapeRadius * EscapeRadius;

	//create new file,give it a name and open it in binary mode  
	fopen_s(&fp, filename.c_str(), "wb");
	fprintf_s(fp, "P6\n %s\n %d\n %d\n %d\n", comment.c_str(), config.iXmax, config.iYmax, config.MaxColorComponentValue);

	omp_set_num_threads(In.numThreadsToUse);

	Out.BeginParallelWorkload();

	create_private(double, Cy);
	parallel_for(yLoop, iY, 0, config.iYmax, 1, schedule(Static, 1),
	{
		Cy = config.CyMin + iY * PixelHeight;
		if (fabs(Cy) < PixelHeight / 2) Cy = 0.0;

		int Iteration = 0;
		for (int iX = 0; iX < config.iXmax; iX++)
		{
			Cx = config.CxMin + iX * PixelWidth;
			//initial value of orbit = critical point Z= 0 
			Zx = 0.0;
			Zy = 0.0;
			Zx2 = Zx * Zx;
			Zy2 = Zy * Zy;

			for (Iteration = 0; Iteration < config.IterationMax && ((Zx2 + Zy2) < ER2); Iteration++)
			{
				Zy = 2 * Zx*Zy + Cy;
				Zx = Zx2 - Zy2 + Cx;
				Zx2 = Zx * Zx;
				Zy2 = Zy * Zy;
			};
			//compute  pixel color (24 bit = 3 bytes) 
			if (Iteration == config.IterationMax)
			{ // interior of Mandelbrot set = black 
				Image[(iY * config.iXmax) + iX + 0] = 0;
				Image[(iY * config.iXmax) + iX + 1] = 0;
				Image[(iY * config.iXmax) + iX + 2] = 0;
			}
			else
			{ //exterior of Mandelbrot set = white 
				Image[(iY * config.iXmax) + iX + 0] = 255; //Red
				Image[(iY * config.iXmax) + iX + 1] = 255; //Green 
				Image[(iY * config.iXmax) + iX + 2] = 255;//Blue 
			};
		}
	});

	Out.BeginResourceCleanup();

	fwrite(Image, 1, config.iXmax * config.iYmax * 3, fp);
	fclose(fp);
	delete Image;

	Out.EndTask(true);
}

void MandelbrotTest::DoOpenMP(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();

	//image data
	unsigned char* Image = new unsigned char[config.iYmax * config.iXmax * 3];

	double PixelWidth = (config.CxMax - config.CxMin) / config.iXmax;
	double PixelHeight = (config.CyMax - config.CyMin) / config.iYmax;

	FILE* fp;
	std::string filename = "ompA.ppm";
	std::string comment = "# ";//comment should start with #

	double Cx, Cy;
	double Zx, Zy;
	double Zx2, Zy2;

	//bail-out value , radius of circle ;  
	const double EscapeRadius = 2;
	double ER2 = EscapeRadius * EscapeRadius;

	//create new file,give it a name and open it in binary mode  
	fopen_s(&fp, filename.c_str(), "wb");
	fprintf_s(fp, "P6\n %s\n %d\n %d\n %d\n", comment.c_str(), config.iXmax, config.iYmax, config.MaxColorComponentValue);

	omp_set_num_threads(In.numThreadsToUse);

	Out.BeginParallelWorkload();

	#pragma omp parallel for private(Cy)
	for (int iY = 0; iY < config.iYmax; iY++)
	{
		Cy = config.CyMin + iY * PixelHeight;
		if (fabs(Cy) < PixelHeight / 2) Cy = 0.0;

		int Iteration = 0;
		for (int iX = 0; iX < config.iXmax; iX++)
		{
			Cx = config.CxMin + iX * PixelWidth;
			//initial value of orbit = critical point Z= 0 
			Zx = 0.0;
			Zy = 0.0;
			Zx2 = Zx * Zx;
			Zy2 = Zy * Zy;

			for (Iteration = 0; Iteration < config.IterationMax && ((Zx2 + Zy2) < ER2); Iteration++)
			{
				Zy = 2 * Zx*Zy + Cy;
				Zx = Zx2 - Zy2 + Cx;
				Zx2 = Zx * Zx;
				Zy2 = Zy * Zy;
			};
			//compute  pixel color (24 bit = 3 bytes) 
			if (Iteration == config.IterationMax)
			{ // interior of Mandelbrot set = black 
				Image[(iY * config.iXmax) + iX + 0] = 0;
				Image[(iY * config.iXmax) + iX + 1] = 0;
				Image[(iY * config.iXmax) + iX + 2] = 0;
			}
			else
			{ //exterior of Mandelbrot set = white 
				Image[(iY * config.iXmax) + iX + 0] = 255; //Red
				Image[(iY * config.iXmax) + iX + 1] = 255; //Green 
				Image[(iY * config.iXmax) + iX + 2] = 255;//Blue 
			};
		}
	}

	Out.BeginResourceCleanup();
	
	fwrite(Image, 1, config.iXmax * config.iYmax * 3, fp);
	fclose(fp);
	delete Image;

	Out.EndTask(true);
}

void MandelbrotTest::DoBoost(const TestParams & In, RetryResult & Out)
{
	throw;
}
