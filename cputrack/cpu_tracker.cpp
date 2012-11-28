/*

CPU only tracker

*/

#pragma warning(disable: 4996) // Function call with parameters that may be unsafe

#include "cpu_tracker.h"
#include "LsqQuadraticFit.h"
#include "random_distr.h"
#include <exception>

const float XCorScale = 1.0f; // keep this at 1, because linear oversampling was obviously a bad idea..

static int round(xcor_t f) { return (int)(f+0.5f); }
template<typename T>
T conjugate(const T &v) { return T(v.real(),-v.imag()); }

CPUTracker::CPUTracker(int w, int h, int xcorwindow)
{
	width = w;
	height = h;

	xcorBuffer = 0;
	
	srcImage = new float [w*h];
	debugImage = new float [w*h];
	std::fill(srcImage, srcImage+w*h, 0.0f);
	std::fill(debugImage, debugImage+w*h, 0.0f);

	zluts = 0;
	zlut_planes = zlut_res = zlut_count = zlut_angularSteps = 0;
	zlut_minradius = zlut_maxradius = 0.0f;
	zlut_compareFourier = true;
	xcorw = xcorwindow;

	qi_radialsteps = 0;
	qi_fft_forward = qi_fft_backward = 0;
}

CPUTracker::~CPUTracker()
{
	delete[] srcImage;
	delete[] debugImage;
	if (zluts && zlut_memoryOwner) 
		delete[] zluts;

	if (xcorBuffer)
		delete xcorBuffer;

	if (qi_fft_forward) { 
		delete qi_fft_forward;
		delete qi_fft_backward;
	}
}

void CPUTracker::SetImageFloat(float *src) {
	for (int k=0;k<width*height;k++)
		srcImage[k]=src[k];
}


#ifdef _DEBUG
	#define MARKPIXEL(x,y) (debugImage[ (int)(y)*width+ (int) (x)]+=maxImageValue*0.1f)
	#define MARKPIXELI(x,y) _markPixels(x,y,debugImage, width, maxImageValue*0.1f);
static void _markPixels(float x,float y, float* img, int w, float mv)
{
	img[ (int)floorf(y)*w+(int)floorf(x) ] += mv;
	img[ (int)floorf(y)*w+(int)ceilf(x) ] += mv;
	img[ (int)ceilf(y)*w+(int)floorf(x) ] += mv;
	img[ (int)ceilf(y)*w+(int)ceilf(x) ] += mv;
}
#else
	#define MARKPIXEL(x,y)
	#define MARKPIXELI(x,y)
#endif

XCor1DBuffer::XCor1DBuffer(int xcorw) 
	: fft_forward(xcorw, false), fft_backward(xcorw, true)
{
	X_xcr.resize(xcorw);
	Y_xcr.resize(xcorw);
	X_xc.resize(xcorw);
	X_result.resize(xcorw);
	Y_xc.resize(xcorw);
	Y_result.resize(xcorw);
	shiftedResult.resize(xcorw);
	this->xcorw = xcorw;

	fft_out = new complexc[xcorw];
	fft_revout = new complexc[xcorw];
}

XCor1DBuffer::~XCor1DBuffer()
{
	delete[] fft_out;
	delete[] fft_revout;
}



void XCor1DBuffer::OutputDebugInfo()
{
	for (int i=0;i<xcorw;i++) {
		//dbgout(SPrintf("i=%d,  X = %f;  X_rev = %f;  Y = %f,  Y_rev = %f\n", i, X_xc[i], X_xcr[i], Y_xc[i], Y_xcr[i]));
		dbgout(SPrintf("i=%d,  X_result = %f;   X = %f;  X_rev = %f\n", i, X_result[i], X_xc[i], X_xcr[i]));
	}
}



void XCor1DBuffer::XCorFFTHelper(complexc* xc, complexc *xcr, xcor_t* result)
{
	fft_forward.transform(xc, fft_out);
	fft_forward.transform(xcr, fft_revout);

	// Multiply with conjugate of reverse
	for (int x=0;x<xcorw;x++) {
		fft_out[x] *= conjugate(fft_revout[x]);
	}

	fft_backward.transform(fft_out, &shiftedResult[0]);

	for (int x=0;x<xcorw;x++)
		result[x] = shiftedResult[ (x+xcorw/2) % xcorw ].real();
}

// Returns true if bounds are crossed
bool CPUTracker::KeepInsideBoundaries(vector2f* center, float radius)
{
	bool boundaryHit = center->x + radius >= width ||
		center->x - radius < 0 ||
		center->y + radius >= height ||
		center->y - radius < 0;

	if (center->x - radius < 0.0f)
		center->x = radius;

	if (center->y - radius < 0.0f)
		center->y = radius;

	if (center->x + radius >= width)
		center->x = width-radius-1;

	if (center->y + radius >= height)
		center->y = height-radius-1;
	return boundaryHit;
}

vector2f CPUTracker::ComputeXCorInterpolated(vector2f initial, int iterations, int profileWidth, bool& boundaryHit)
{
	// extract the image
	vector2f pos = initial;

	if (!xcorBuffer)
		xcorBuffer = new XCor1DBuffer(xcorw);

	if (xcorw < profileWidth)
		profileWidth = xcorw;

#ifdef _DEBUG
	std::copy(srcImage, srcImage+width*height, debugImage);
	maxImageValue = *std::max_element(srcImage,srcImage+width*height);
#endif

	if (xcorw > width || xcorw > height) {
		boundaryHit = true;
		return initial;
	}

	boundaryHit = false;
	for (int k=0;k<iterations;k++) {
		boundaryHit = KeepInsideBoundaries(&pos, XCorScale*xcorw/2);

		float xmin = pos.x - XCorScale * xcorw/2;
		float ymin = pos.y - XCorScale * xcorw/2;

		complexc* xc = &xcorBuffer->X_xc[0];
		complexc* xcr = &xcorBuffer->X_xcr[0];
		// generate X position xcor array (summing over y range)
		for (int x=0;x<xcorw;x++) {
			xcor_t s = 0.0f;
			for (int y=0;y<profileWidth;y++) {
				float xp = x * XCorScale + xmin;
				float yp = pos.y + XCorScale * (y - profileWidth/2);
				s += Interpolate(srcImage, width, height, xp, yp);
				MARKPIXELI(xp, yp);
			}
			xc [x] = s;
			xcr [xcorw-x-1] = xc[x];
		}

		xcorBuffer->XCorFFTHelper(xc, xcr, &xcorBuffer->X_result[0]);
		xcor_t offsetX = ComputeMaxInterp(&xcorBuffer->X_result[0],xcorBuffer->X_result.size()) - (xcor_t)xcorw/2;

		// generate Y position xcor array (summing over x range)
		xc = &xcorBuffer->Y_xc[0];
		xcr = &xcorBuffer->Y_xcr[0];
		for (int y=0;y<xcorw;y++) {
			xcor_t s = 0.0f; 
			for (int x=0;x<profileWidth;x++) {
				float xp = pos.x + XCorScale * (x - profileWidth/2);
				float yp = y * XCorScale + ymin;
				s += Interpolate(srcImage,width,height, xp, yp);
				MARKPIXELI(xp,yp);
			}
			xc[y] = s;
			xcr [xcorw-y-1] = xc[y];
		}

		xcorBuffer->XCorFFTHelper(xc,xcr, &xcorBuffer->Y_result[0]);
		xcor_t offsetY = ComputeMaxInterp(&xcorBuffer->Y_result[0], xcorBuffer->Y_result.size()) - (xcor_t)xcorw/2;

		pos.x += (offsetX - 1) * XCorScale * 0.5f;
		pos.y += (offsetY - 1) * XCorScale * 0.5f;
	}

	return pos;
}




vector2f CPUTracker::ComputeQI(vector2f initial, int iterations, int radialSteps, int angularStepsPerQ, float minRadius, float maxRadius, bool& boundaryHit)
{
	int nr=radialSteps;
#ifdef _DEBUG
	std::copy(srcImage, srcImage+width*height, debugImage);
	maxImageValue = *std::max_element(srcImage,srcImage+width*height);
#endif

	if (angularStepsPerQ != quadrantDirs.size()) {
		quadrantDirs.resize(angularStepsPerQ);
		for (int j=0;j<angularStepsPerQ;j++) {
			float ang = 0.5*3.141593f*j/(float)angularStepsPerQ;
			vector2f d = { cosf(ang), sinf(ang) };
			quadrantDirs[j] = d;
		}
	}
	if(!qi_fft_forward || qi_radialsteps != nr) {
		if(qi_fft_forward) {
			delete qi_fft_forward;
			delete qi_fft_backward;
		}
		qi_radialsteps = nr;
		qi_fft_forward = new kissfft<qi_t>(nr*2,false);
		qi_fft_backward = new kissfft<qi_t>(nr*2,true);
	}

	qi_t* buf = (qi_t*)ALLOCA(sizeof(qi_t)*nr*4);
	qi_t* q0=buf, *q1=buf+nr, *q2=buf+nr*2, *q3=buf+nr*3;
	qic_t* concat0 = (qic_t*)ALLOCA(sizeof(qic_t)*nr*2);
	qic_t* concat1 = concat0 + nr;

	vector2f center = initial;

	float pixelsPerProfLen = (maxRadius-minRadius)/radialSteps;
	boundaryHit = false;

	if (width < maxRadius || height < maxRadius) {
		boundaryHit = true;
		return initial;
	}

	for (int k=0;k<iterations;k++){
		// check bounds
		boundaryHit = KeepInsideBoundaries(&center, maxRadius);

		for (int q=0;q<4;q++) {
			ComputeQuadrantProfile(buf+q*nr, nr, angularStepsPerQ, q, minRadius, maxRadius, center);
		}
		
		// Build Ix = qL(-r) || qR(r)
		// qL = q1 + q2   (concat0)
		// qR = q0 + q3   (concat1)
		for(int r=0;r<nr;r++) {
			concat0[nr-r-1] = q1[r]+q2[r];
			concat1[r] = q0[r]+q3[r];
		}

		float offsetX = QI_ComputeOffset(concat0, nr);

		// Build Iy = qB(-r) || qT(r)
		// qT = q0 + q1
		// qB = q2 + q3
		for(int r=0;r<nr;r++) {
			concat0[r] = q0[r]+q1[r];
			concat1[nr-r-1] = q2[r]+q3[r];
		}
		float offsetY = QI_ComputeOffset(concat0, nr);

		//dbgprintf("[%d] OffsetX: %f, OffsetY: %f\n", k, offsetX, offsetY);

		center.x += offsetX * pixelsPerProfLen;
		center.y += offsetY * pixelsPerProfLen;
	}

	return center;
}


// Profile is complexc[nr*2]
CPUTracker::qi_t CPUTracker::QI_ComputeOffset(CPUTracker::qic_t* profile, int nr)
{
	qic_t* reverse = ALLOCA_ARRAY(qic_t, nr*2);
	qic_t* fft_out = ALLOCA_ARRAY(qic_t, nr*2);
	qic_t* fft_out2 = ALLOCA_ARRAY(qic_t, nr*2);

	for(int x=0;x<nr*2;x++)
		reverse[x] = profile[nr*2-1-x];

	qi_fft_forward->transform(profile, fft_out);
	qi_fft_forward->transform(reverse, fft_out2); // fft_out2 contains fourier-domain version of reverse profile

	// multiply with conjugate
	for(int x=0;x<nr*2;x++)
		fft_out[x] *= conjugate(fft_out2[x]);

	qi_fft_backward->transform(fft_out, fft_out2);
	// fft_out2 now contains the autoconvolution
	// convert it to float
	qi_t* autoconv = ALLOCA_ARRAY(qi_t, nr*2);
	for(int x=0;x<nr*2;x++)
		autoconv[x] = fft_out2[(x+nr)%(nr*2)].real();
	float maxPos = ComputeMaxInterp(autoconv, nr*2);
	return (maxPos - nr) / (3.141593 * 0.5);
}


void CPUTracker::ComputeQuadrantProfile(CPUTracker::qi_t* dst, int radialSteps, int angularSteps, int quadrant, float minRadius, float maxRadius, vector2f center)
{
	const int qmat[] = {
		1, 1,
		-1, 1,
		-1, -1,
		1, -1 };
	int mx = qmat[2*quadrant+0];
	int my = qmat[2*quadrant+1];

	for (int i=0;i<radialSteps;i++)
		dst[i]=0.0f;

	float total = 0.0f;
	float rstep = (maxRadius - minRadius) / radialSteps;
	for (int i=0;i<radialSteps; i++) {
		float sum = 0.0f;
		float r = minRadius + rstep * i;

		for (int a=0;a<angularSteps;a++) {
			float x = center.x + mx*quadrantDirs[a].x * r;
			float y = center.y + my*quadrantDirs[a].y * r;
			sum += Interpolate(srcImage,width,height, x,y);
			MARKPIXELI(x,y);
		}

		dst[i] = sum;
		total += dst[i];
	}
	for (int i=0;i<radialSteps;i++)
		dst[i] /= total;
}



vector2f CPUTracker::ComputeBgCorrectedCOM()
{
	float sum=0, sum2=0;
	float momentX=0;
	float momentY=0;

	for (int y=0;y<height;y++)
		for (int x=0;x<width;x++) {
			float v = getPixel(x,y);
			sum += v;
			sum2 += v*v;
		}

	float invN = 1.0f/(width*height);
	float mean = sum * invN;
	float stdev = sqrtf(sum2 * invN - mean * mean);
	sum = 0.0f;

	for (int y=0;y<height;y++)
		for(int x=0;x<width;x++)
		{
			float v = getPixel(x,y);
			v = std::max(0.0f, fabs(v-mean)-2.0f*stdev);
			sum += v;
			momentX += x*v;
			momentY += y*v;
		}
	vector2f com;
	com.x = momentX / (float)sum;
	com.y = momentY / (float)sum;
	return com;
}


void CPUTracker::Normalize(float* d)
{
	if (!d) d=srcImage;
	normalize(d, width, height);
}


void CPUTracker::ComputeRadialProfile(float* dst, int radialSteps, int angularSteps, float minradius, float maxradius, vector2f center, bool* pBoundaryHit)
{
	bool boundaryHit = KeepInsideBoundaries(&center, maxradius);
	if (pBoundaryHit) *pBoundaryHit = boundaryHit;
	ImageData imgData (srcImage, width,height);
	::ComputeRadialProfile(dst, radialSteps, angularSteps, minradius, maxradius, center, &imgData);
}

void CPUTracker::SetZLUT(float* data, int planes, int res, int numLUTs, float minradius, float maxradius, int angularSteps, bool copyMemory, bool compareFourier)
{
	if (zluts && zlut_memoryOwner)
		delete[] zluts;

	if (compareFourier)
		copyMemory = true;
	
	if (copyMemory) {
		zluts = new float[planes*res*numLUTs];
		std::copy(data, data+(planes*res*numLUTs), zluts);
	} else
		zluts = data;
	zlut_memoryOwner = !copyMemory;
	zlut_planes = planes;
	zlut_res = res;
	zlut_count = numLUTs;
	zlut_minradius = minradius;
	zlut_maxradius = maxradius;
	zlut_angularSteps = angularSteps;
	zlut_compareFourier = compareFourier;
	/*
	if (compareFourier) {
		kissfft<float> fw(zlut_res, false);
		std::complex<float>* srcbuf = (std::complex<float>*)ALLOCA(sizeof(std::complex<float>)*zlut_res);
		// convert ZLUT to FD
		for (int i=0;i<zlut_count;i++) {
			float* zlut = &zluts[planes*res*i];
			for (int y = 0; y < zlut_planes; y++) {
				float* rprof = &zlut[res*y];
				std::copy(rprof,rprof+res,srcbuf);
				fw.transform(srcbuf, dstbuf);
			}
		}
	}*/ 

}



float CPUTracker::ComputeZ(vector2f center, int angularSteps, int zlutIndex, bool* boundaryHit, float* profile, std::vector<float>* cmpProf)
{
	if (!zluts)
		return 0.0f;
	
	float* rprof = ALLOCA_ARRAY(float, zlut_res);
	float* rprof_diff = ALLOCA_ARRAY(float, zlut_planes);

	ComputeRadialProfile(rprof, zlut_res, angularSteps, zlut_minradius, zlut_maxradius, center, boundaryHit);
	if (profile) std::copy(rprof, rprof+zlut_res, profile);

	// Now compare the radial profile to the profiles stored in Z

	float* zlut_sel = &zluts[zlut_planes*zlut_res*zlutIndex];

	if (zlut_compareFourier) {
		kissfft<float> fw(zlut_res, false);
		std::complex<float>* srcbuf = ALLOCA_ARRAY(std::complex<float>, zlut_res);
		std::complex<float>* smpbuf = ALLOCA_ARRAY(std::complex<float>, zlut_res); // holding sample in FD
		std::complex<float>* lutcmp = ALLOCA_ARRAY(std::complex<float>, zlut_res);// holding LUT row in FD
		std::copy(rprof, rprof+zlut_res, srcbuf);
		fw.transform(srcbuf, smpbuf);
		for (int k=0;k<zlut_planes;k++) {
			std::copy(zlut_sel,zlut_sel+zlut_res,srcbuf);
			fw.transform(srcbuf, lutcmp);
			float diffsum = 0.0f;
			for (int r = 0; r<zlut_res;r++) {
				std::complex<float> diff = lutcmp[r]-smpbuf[r];
				diffsum += fabs(diff.real());//(diff*conjugate(diff)).real();
			}
			rprof_diff[k] = -diffsum;
		}
	} else {
		for (int k=0;k<zlut_planes;k++) {
			float diffsum = 0.0f;
			for (int r = 0; r<zlut_res;r++) {
				float diff = rprof[r]-zlut_sel[k*zlut_res+r];
				diffsum += diff*diff;
			}
			rprof_diff[k] = -diffsum;
		}
	}

	if (cmpProf) {
		cmpProf->resize(zlut_planes);
		std::copy(rprof_diff, rprof_diff+zlut_planes, cmpProf->begin());
	}

	float z = ComputeMaxInterp(rprof_diff, zlut_planes);
	return z;
}

static void CopyCpxVector(std::vector<xcor_t>& xprof, const std::vector<complexc>& src) {
	xprof.resize(src.size());
	for (int k=0;k<src.size();k++)
		xprof[k]=src[k].real();
}

bool CPUTracker::GetLastXCorProfiles(std::vector<xcor_t>& xprof, std::vector<xcor_t>& yprof, 
		std::vector<xcor_t>& xconv, std::vector<xcor_t>& yconv)
{
	if (xcorBuffer) {
		CopyCpxVector(xprof, xcorBuffer->X_xc);
		CopyCpxVector(yprof, xcorBuffer->Y_xc);
		xconv = xcorBuffer->X_result;
		yconv = xcorBuffer->Y_result;
	}
	return true;
}

vector2f CPUTracker::ComputeXCor2D()
{
	vector2f x={};
	return x;
}



