/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2011 Sergey Lisitsyn
 * Copyright (C) 2011 Berlin Institute of Technology and Max-Planck-Society
 */

#include "lib/arpack.h"
#ifdef HAVE_ARPACK
#ifdef HAVE_ATLAS
#include "lib/config.h"
#include <cblas.h>
#include "lib/lapack.h"
#include "lib/common.h"
#include "lib/io.h"
#include <string.h>

using namespace shogun;

namespace shogun
{
void arpack_dsaupd(double* matrix, int n, int nev, const char* which, 
                   int mode, double shift,
                   double* eigenvalues, double* eigenvectors, int& status)
{
	// check if nev is greater than n
	if (nev>n)
		SG_SERROR("Number of required eigenpairs is greater than order of the matrix");

	// check specified mode
	if (mode!=1 && mode!=2)
		SG_SERROR("Unknown mode specified");

	// init ARPACK's reverse communication parameter 
 	// (should be zero initially)
	int ido = 0;

	// specify that non-general eigenproblem will be solved
	// (Ax=lGx, where G=I)
	char bmat[2] = "I";

	// init tolerance (zero means machine precision)
	double tol = 0.0;

	// allocate array to hold residuals
	double* resid = new double[n];

	// set number of Lanczos basis vectors to be used
	// (with max(4*nev,n) sufficient for most tasks)
	int ncv = nev*4>n ? n : nev*4;

	// allocate array 'v' for dsaupd routine usage
	int ldv = n;
	double* v = new double[ldv*ncv];

	// init array for i/o params for routine
	int* iparam = new int[11];
	// specify method for selecting implicit shifts (1 - exact shifts) 
	iparam[0] = 1;
	// specify max number of iterations
	iparam[2] = 3*n;
	// set the computation mode (1 for regular or 2 for shift-inverse)
	iparam[6] = mode;

	// init array indicating locations of vectors for routine callback
	int* ipntr = new int[11];

	// allocate workaround arrays
	double* workd = new double[3*n];
	int lworkl = ncv*(ncv+8);
	double* workl = new double[lworkl];

	// init info holding status (should be zero at first call)
	int info = 0;

	// which eigenpairs to find
	char* which_ = strdup(which);
	// All
	char* all_ = strdup("All");

	// main computation loop
	// shift-invert mode
	if (mode==2)
	{
		double* workt = new double[n];		

		for (int i=0; i<n; i++)
			matrix[i*n+i] -= shift;

		clapack_dpotri(CblasColMajor,CblasUpper,n,matrix,n);

		do 
		{
			dsaupd_(&ido, bmat, &n, which_, &nev, &tol, resid,
			        &ncv, v, &ldv, iparam, ipntr, workd, workl,
			        &lworkl, &info);

			if ((ido==1)||(ido==-1))
			{
				memcpy(workt,workd+ipntr[0]-1,n*sizeof(double));

				cblas_dtrmv(CblasColMajor,CblasUpper,CblasNoTrans,CblasNonUnit,
				            n,matrix,n,
				            workt,1);

				memcpy(workd+ipntr[1]-1,workt,n*sizeof(double));
			}
		} while ((ido==1)||(ido==-1));
	} 
	// regular mode
	if (mode==1)
	{
		do	 
		{
			dsaupd_(&ido, bmat, &n, which_, &nev, &tol, resid,
		        	&ncv, v, &ldv, iparam, ipntr, workd, workl,
		        	&lworkl, &info);

			if ((ido==1)||(ido==-1))
			{
				cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans,
			        	    n, 1, n,
					    1.0, matrix, n,
				            (workd+ipntr[0]-1), n,
				            0.0, (workd+ipntr[1]-1), n);		
			}
		} while ((ido==1)||(ido==-1));
	
	}

	// check if DSAUPD failed
	if (info<0) 
	{
		if ((info<=-1)&&(info>=-6))
			SG_SWARNING("DSAUPD failed. Wrong parameter passed.");
		else if (info==-7)
			SG_SWARNING("DSAUPD failed. Workaround array size is not sufficient.");
		else 
			SG_SWARNING("DSAUPD failed. Error code: %d.", info);

		status = -1;
	}
	else 
	{
		SG_SDEBUG("DSAUPD finished. Taken %d iterations.\n", iparam[2]);
		if (info==1)
			SG_SDEBUG("Maximum number of iterations reached.\n");
			
		// allocate select for dseupd
		int* select = new int[ncv];
		// allocate d to hold eigenvalues
		double* d = new double[2*ncv];
		// sigma for dseupd
		double sigma;
		
		// init ierr indicating dseupd possible errors
		int ierr = 0;

		// specify that eigenvectors to be computed too		
		int rvec = 1;

		dseupd_(&rvec, all_, select, d, v, &ldv, &sigma, bmat,
		        &n, which_, &nev, &tol, resid, &ncv, v, &ldv,
		        iparam, ipntr, workd, workl, &lworkl, &ierr);

		if (ierr!=0)
		{
			SG_SWARNING("DSEUPD failed with status=%d", ierr);
			status = -1;
		}
		else
		{
		
			for (int i=0; i<nev; i++)
			{	
				eigenvalues[i] = d[i];
			
				for (int j=0; j<n; j++)
					eigenvectors[j*nev+i] = v[i*n+j];
			}
		}
		
		// cleanup
		delete[] select;
		delete[] d;
	}

	// cleanup
	delete[] all_;
	delete[] which_;
	delete[] resid;
	delete[] v;
	delete[] iparam;
	delete[] ipntr;
	delete[] workd;
	delete[] workl;
};

}
#endif /* HAVE_ATLAS */
#endif /* HAVE_ARPACK */