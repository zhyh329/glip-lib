/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-COMPUTE                                                                                              */
/*     Command-Line Utility for the OpenGL Image Processing LIBrary                                              */
/*                                                                                                               */
/*     Author        : R. KERVICHE (ronan.kerviche@free.fr)                                                      */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : http://sourceforge.net/projects/glip-lib/                                                 */
/*                                                                                                               */
/*     File          : GlipCompute.hpp                                                                           */
/*     Original Date : August 18th 2014                                                                          */
/*                                                                                                               */
/*     Description   : FreeImage3 interface for image input/output.                                              */
/*                                                                                                               */
/* ************************************************************************************************************* */

#ifndef __GLIPCOMPUTE__
#define __GLIPCOMPUTE__

	// Includes : 
	#include "CreateWindowlessContext.hpp"
	#include "DeviceMemoryManager.hpp"

	struct ProcessCommand
	{
		int							line;
		std::string						name;
		std::vector< std::pair<std::string, std::string> >	inputFilenames,
									outputFilenames;
		int							uniformsLine;
		std::string						uniformVariables;

		ProcessCommand(void);
	};

	extern int parseArguments(int argc, char** argv, std::string& pipelineFilename, size_t& memorySize, std::string& inputFormatString, std::vector<ProcessCommand>& commands);
	extern int compute(const std::string& pipelineFilename, const size_t& memorySize, const std::string& inputFormatString, const std::vector<ProcessCommand>& commands);

#endif
