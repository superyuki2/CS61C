// CS 61C Fall 2015 Project 4

// include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif

// include OpenMP
#if !defined(_MSC_VER)
#include <pthread.h>
#endif
#include <omp.h>
#include <string.h>

#include "calcDepthOptimized.h"
#include "calcDepthNaive.h"

/* DO NOT CHANGE ANYTHING ABOVE THIS LINE. */

void calcDepthOptimized(float *depth, float *left, float *right, int imageWidth, int imageHeight, int featureWidth, int featureHeight, int maximumDisplacement)
{
	/* The two outer for loops iterate through each pixel */
	memset(depth, 0, imageWidth * imageHeight * sizeof(float));
	int unroll = (2 * featureWidth + 1)/4 * 4;
	int tail = 2 * featureWidth + 1;
	#pragma omp parallel for
	for (int y = 0; y < imageHeight; y++)
	{
		for (int x = 0; x < imageWidth; x++)
		{	
			/* Set the depth to 0 if looking at edge of the image where a feature box cannot fit. */
			if ((y < featureHeight) || (y >= imageHeight - featureHeight) || (x < featureWidth) || (x >= imageWidth - featureWidth))
			{
				depth[y * imageWidth + x] = 0;
				continue;
			}

			float minimumSquaredDifference = -1;
			int minimumDy = 0;
			int minimumDx = 0;
			int lowerBoundx, upperBoundx, lowerBoundy, upperBoundy;
			if (-maximumDisplacement > featureWidth - x) {
				lowerBoundx = -maximumDisplacement;
			}
			else {
				lowerBoundx = featureWidth - x;
			}
			if (maximumDisplacement < imageWidth - featureWidth - x - 1) {
				upperBoundx = maximumDisplacement;
			}
			else {
				upperBoundx = imageWidth - featureWidth - x - 1;
			}
			if (-maximumDisplacement > featureHeight - y) {
				lowerBoundy = -maximumDisplacement;
			}
			else {
				lowerBoundy = featureHeight - y;
			}
			if (maximumDisplacement < imageHeight - featureHeight - y - 1) {
				upperBoundy = maximumDisplacement;
			}
			else {
				upperBoundy = imageHeight - featureHeight - y - 1;
			}

			/* Iterate through all feature boxes that fit inside the maximum displacement box. 
			   centered around the current pixel. */ 
			for (int dy = lowerBoundy; dy <= upperBoundy; dy++)
			{
				for (int dx = lowerBoundx; dx <= upperBoundx; dx++)
				{
					float squaredDifference = 0;
					float total[4];
					__m128 tempSum = _mm_setzero_ps();
					__m128 leftV;
					__m128 rightV;
					__m128 diffV;
					__m128 squareV;

					//Unroll by 4
					for (int boxX = 0; boxX < unroll; boxX += 4) {
						for (int boxY = -featureHeight; boxY <= featureHeight; boxY++) {
							int leftI = (x + boxX - featureWidth) + imageWidth * (y + boxY);
							int rightI = (x + dx + boxX - featureWidth) + imageWidth * (y + dy + boxY);
							leftV = _mm_loadu_ps(leftI + left);
							rightV = _mm_loadu_ps(rightI + right);
							diffV = _mm_sub_ps(leftV, rightV);
							squareV = _mm_mul_ps(diffV, diffV);
							tempSum = _mm_add_ps(squareV, tempSum);
						}
					}

					/* Sum the squared difference within a box of +/- featureHeight and +/- featureWidth. */
					for (int boxX = unroll; boxX < tail; boxX++)
					{
						for (int boxY = -featureHeight; boxY <= featureHeight; boxY++)
						{
							int leftX = x + boxX - featureWidth;
							int leftY = y + boxY;
							int rightX = x + dx + boxX - featureWidth;
							int rightY = y + dy + boxY;

							float difference = left[leftY * imageWidth + leftX] - right[rightY * imageWidth + rightX];
							squaredDifference += difference * difference;
						}
					}

					_mm_storeu_ps(total, tempSum);
					squaredDifference += total[0] + total[1] + total[2] + total[3];

					/* 
					Check if you need to update minimum square difference. 
					This is when either it has not been set yet, the current
					squared displacement is equal to the min and but the new
					displacement is less, or the current squared difference
					is less than the min square difference.
					*/

					if ((minimumSquaredDifference == -1) || ((minimumSquaredDifference == squaredDifference) && (displacementNaive(dx, dy) < displacementNaive(minimumDx, minimumDy))) || (minimumSquaredDifference > squaredDifference))
					{
						minimumSquaredDifference = squaredDifference;
						minimumDx = dx;
						minimumDy = dy;
					}
				}
			}

			/* 
			Set the value in the depth map. 
			If max displacement is equal to 0, the depth value is just 0.
			*/
			if (minimumSquaredDifference != -1)
			{
				if (maximumDisplacement == 0)
				{
					depth[y * imageWidth + x] = 0;
				}
				else
				{
					depth[y * imageWidth + x] = displacementNaive(minimumDx, minimumDy);
				}
			}
			else
			{
				depth[y * imageWidth + x] = 0;
			}
		}
	}
}


