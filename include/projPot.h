//
// Created by AJ Pryor on 2/22/17.
//

#ifndef PRISM_PROJPOT_H
#define PRISM_PROJPOT_H
#include "ArrayND.h"
#include "boost/math/special_functions/bessel.hpp"
#include <math.h>
#include <iostream>
#include <algorithm>
#include <numeric>
#include "fparams.h"
namespace PRISM {

	template <class T>
	T get_potMin(const ArrayND<2, std::vector<T> >& pot,
	             const ArrayND<1, std::vector<T> >& xr,
	             const ArrayND<1, std::vector<T> >& yr){
		// I am assuming that xr and yr are symmetric about 0
		const size_t xInd = std::floor(xr.size()/2);
		const size_t yInd = std::floor(yr.size()/2);
		const T dx        = round(sqrt(2*xInd - 1));
		const T dy        = round(sqrt(2*yInd - 1));
		const T xv[]      = {xInd-dx-1, xInd+dx+1, xInd-dx-1, xInd+dx+1, 0, 0, (T)xr.size()-1, (T)xr.size()-1};
		const T yv[]      = {0, 0, (T)yr.size()-1, (T)yr.size()-1, yInd-dy-1, yInd+dy+1, yInd-dy-1, yInd+dy+1};

		T potMin = 0;
		for (auto i=0; i < 8; ++i)potMin = (pot.at(yv[i],xv[i]) > potMin) ? pot.at(yv[i],xv[i]) : potMin;
		return potMin;
	}

	using namespace std;
	template <class T>
	using Array2D = PRISM::ArrayND<2, std::vector<T> >;
	template <class T>
	using Array1D = PRISM::ArrayND<1, std::vector<T> >;

	template<class T>
	ArrayND<2, std::vector<T> >
	projPot(const size_t &Z, const ArrayND<1, std::vector<T> > &xr, const ArrayND<1, std::vector<T> > &yr) {
		static const T pi = std::acos(-1);
		T ss    = 8;
		T a0    = 0.5292;
		T e     = 14.4;
		T term1 = 4*pi*pi*a0*e;
		T term2 = 2*pi*pi*a0*e;
		ArrayND<2, std::vector<T> > result = zeros_ND<2, T>({{yr.size(), xr.size()}});
		const T dx = xr[1] - xr[0];
		const T dy = yr[1] - yr[0];

		T start = -(ss-1)/ss/2;
		const T step  = 1/ss;
		const T end   = -start;
		vector<T> sub_data;
		while (start <= end){
			sub_data.push_back(start);
			start+=step;
		}
		ArrayND<1, std::vector<T> > sub(sub_data,{{sub_data.size()}});

		std::pair<Array2D<T>, Array2D<T> > meshx = meshgrid(xr, sub*dx);
		std::pair<Array2D<T>, Array2D<T> > meshy = meshgrid(yr, sub*dy);

		ArrayND<1, std::vector<T> > xv = zeros_ND<1, T>({{meshx.first.size()}});
		ArrayND<1, std::vector<T> > yv = zeros_ND<1, T>({{meshy.first.size()}});
		{
			auto t_x = xv.begin();
			for (auto i = 0; i < meshx.first.get_dimi(); ++i) {
				for (auto j = 0; j < meshx.first.get_dimj(); ++j) {
					*t_x++ = meshx.first.at(j, i) + meshx.second.at(j, i);
				}
			}
		}

		{
			auto t_y = yv.begin();
			for (auto i = 0; i < meshy.first.get_dimi(); ++i) {
				for (auto j = 0; j < meshy.first.get_dimj(); ++j) {
					*t_y++ = meshy.first.at(j, i) + meshy.second.at(j, i);
				}
			}
		}

		std::pair<Array2D<T>, Array2D<T> > meshxy = meshgrid(xv, yv);
		ArrayND<2, std::vector<T> > r2 = zeros_ND<2, T>({{yv.size(), xv.size()}});
		ArrayND<2, std::vector<T> > r  = zeros_ND<2, T>({{yv.size(), xv.size()}});

		{
			auto t_y = r2.begin();
			for (auto i = 0; i < meshxy.first.get_dimi(); ++i) {
				for (auto j = 0; j < meshxy.first.get_dimj(); ++j) {
					*t_y++ = pow(meshxy.first.at(j,i),2) + pow(meshxy.second.at(j,i),2);
				}
			}
		}


		for (auto i = 0; i < r.size(); ++i)r[i] = sqrt(r2[i]);
		// construct potential
		ArrayND<2, std::vector<T> > potSS  = ones_ND<2, T>({{r2.get_dimj(), r2.get_dimi()}});
		std::vector<double> ap;
		ap.reserve(n_parameters);
		for (auto i = 0; i < n_parameters; ++i){
			ap[i] = fparams[(Z-1)*n_parameters + i];
		}

		using namespace boost::math;
		std::transform(r.begin(), r.end(),
		          r2.begin(), potSS.begin(), [&ap, &term1, &term2](const T& r_t, const T& r2_t){

			return term1*(ap[0] *
	                cyl_bessel_k(0,2*pi*sqrt(ap[1])*r_t)          +
					ap[2]*cyl_bessel_k(0,2*pi*sqrt(ap[3])*r_t)    +
					ap[4]*cyl_bessel_k(0,2*pi*sqrt(ap[5])*r_t))   +
					term2*(ap[6]/ap[7]*exp(-pow(pi,2)/ap[7]*r2_t) +
					ap[8]/ap[9]*exp(-pow(pi,2)/ap[9]*r2_t)        +
					ap[10]/ap[11]*exp(-pow(pi,2)/ap[11]*r2_t));
		});

		// integrate
		ArrayND<2, std::vector<T> > pot = zeros_ND<2, T>({{yr.size(), xr.size()}});
		for (auto sy = 0; sy < ss; ++sy){
			for (auto sx = 0; sx < ss; ++sx) {
				for (auto j = 0; j < pot.get_dimj(); ++j) {
					for (auto i = 0; i < pot.get_dimi(); ++i) {
						pot.at(j, i) += potSS.at(j*ss + sy, i*ss + sx);
					}
				}
			}
		}
		pot/=(ss*ss);

		T potMin = get_potMin(pot,xr,yr);
		pot -= potMin;
		transform(pot.begin(),pot.end(),pot.begin(),[](T& a){return a<0?0:a;});

		return pot;
	}
}
#endif //PRISM_PROJPOT_H