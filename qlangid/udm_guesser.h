/* Copyright (C) 2000-2008 Lavtech.com corp. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
*/

#ifndef UDM_GUESSER_H
#define UDM_GUESSER_H

#ifdef __cplusplus
extern "C" {
#endif

extern void  UdmBuildLangMap(UDM_LANGMAP * map,const char * text,size_t text_len);
extern void  UdmPrintLangMap(UDM_LANGMAP * map);
extern void  UdmPrepareLangMap(UDM_LANGMAP * map);
extern float UdmCheckLangMap(UDM_LANGMAP * map,UDM_LANGMAP * text);


extern int   UdmLoadLangMapList(UDM_ENV * env, const char * mapdir);
extern int   UdmLoadLangMapFile(UDM_ENV * env, const char * mapname);
extern void  UdmFreeLangMapList(UDM_ENV * env);

#ifdef __cplusplus
}
#endif


#endif
