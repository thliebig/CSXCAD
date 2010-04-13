/*
*	Copyright (C) 2008,2009,2010 Thorsten Liebig (Thorsten.Liebig@gmx.de)
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU Lesser General Public License as published
*	by the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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

// declare a parameter as unused
#define UNUSED(x) (void)(x);

#endif /*CSXCAD_GLOBAL_H_*/
