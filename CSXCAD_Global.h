#ifndef CSXCAD_GLOBAL_H_
#define CSXCAD_GLOBAL_H_
/*
 * Author:	Thorsten Liebig
 * Date:	03-12-2008
 * Lib:		CSXCAD
 * Version:	0.1.2a
 */

#define _CSXCAD_LIB_NAME_ "CSXCAD-Lib: Continuous Structure XML - CAD"
#define _CSXCAD_AUTHOR_ "Thorsten Liebig (2008-2009)"
#define _CSXCAD_AUTHOR_MAIL_ "Thorsten.Liebig@gmx.de"
#define _CSXCAD_VERSION_ "0.1.1 alpha"

#if defined(WIN32)
	#ifdef BUILD_CSXCAD_LIB
	#define CSXCAD_EXPORT __declspec(dllexport)
	#else
	#define CSXCAD_EXPORT __declspec(dllimport)
	#endif
#else
#define CSXCAD_EXPORT
#endif

#endif /*CSXCAD_GLOBAL_H_*/
