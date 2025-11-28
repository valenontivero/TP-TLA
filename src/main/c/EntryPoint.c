#include "backend/code-generation/AudioConverter.h"
#include "backend/code-generation/Generator.h"
#include "backend/code-generation/MidiGenerator.h"
#include "backend/domain-specific/Calculator.h"
#include "backend/semantic-analysis/SemanticAnalyzer.h"
#include "backend/semantic-analysis/SymbolTable.h"
#include "backend/semantic-analysis/TypeChecker.h"
#include "frontend/Frontend.h"
#include "frontend/ImportResolver.h"
#include "frontend/lexical-analysis/FlexActions.h"
#include "frontend/syntactic-analysis/BisonActions.h"
#include "support/logging/Logger.h"
#include "support/type/CompilationStatus.h"
#include "support/type/CompilerState.h"
#include "support/type/ModuleDestructor.h"
#include <string.h>

/**
 * The main entry-point of the entire application. If you use "strtok" to
 * parse anything inside this project instead of using Flex and Bison, I will
 * find you, and I will kill you (Bryan Mills; "Taken", 2008).
 */
const int main(const int length, const char ** arguments) {
	LexicalAnalyzer * lexicalAnalyzer = createLexicalAnalyzer();
	Logger * logger = createLogger("EntryPoint");
	for (int k = 0; k < length; ++k) {
		logDebugging(logger, "Argument %d: \"%s\"", k, arguments[k]);
	}
	CompilerState compilerState = {
		.abstractSyntaxtTree = NULL,
		.value = 0,
		.outputWav = false,
		.outputFlac = false,
		.outputMp3 = false
	};

	/* Parse line command argument -fo=... for output format */
	for (int k = 0; k < length; ++k) {
		const char * argument = arguments[k];
		if (strncmp(argument, "-fo=", 4) == 0) {
			const char * formats = argument + 4;
			logDebugging(logger, "Output formats option detected: \"%s\"", formats);

			compilerState.outputWav = false;
			compilerState.outputFlac = false;
			compilerState.outputMp3 = false;

			const char * start = formats;
			while (*start != '\0') {
				const char * end = start;
				while (*end != '\0' && *end != ',') {
					++end;
				}

				size_t lengthToken = (size_t)(end - start);
				if (lengthToken > 0) {
					if (lengthToken == 3 && strncmp(start, "wav", 3) == 0) {
						compilerState.outputWav = true;
					}
					else if (lengthToken == 3 && strncmp(start, "mp3", 3) == 0) {
						compilerState.outputMp3 = true;
					}
					else if (lengthToken == 4 && strncmp(start, "flac", 4) == 0) {
						compilerState.outputFlac = true;
					}
					else {
						logWarning(logger, "Unknown output format in -fo option: \"%.*s\"", (int)lengthToken, start);
					}
				}

				if (*end == ',') {
					start = end + 1;
				} else {
					break;
				}
			}
		}
	}
	ModuleDestructor moduleDestructors[] = {
		initializeAbstractSyntaxTreeModule(),
		initializeFlexActionsModule(lexicalAnalyzer),
		initializeBisonActionsModule(&compilerState),
		initializeFrontendModule(lexicalAnalyzer),
		initializeImportResolverModule(),
		initializeSymbolTableModule(),
		initializeTypeCheckerModule(),
		initializeSemanticAnalyzerModule(),
		initializeMidiGeneratorModule(),
		initializeAudioConverterModule(),
		initializeCalculatorModule(),
		initializeGeneratorModule()
	};
	CompilationStatus compilationStatus = executeSyntacticAnalysis();
	Program * program = compilerState.abstractSyntaxtTree;
	if (compilationStatus == SUCCEEDED) {
		// ----------------------------------------------------------------------------------------
		// Import Resolution Phase ----------------------------------------------------------------
		logDebugging(logger, "Resolving imports...");
		compilationStatus = resolveImports(program);
		// ----------------------------------------------------------------------------------------
		if (compilationStatus == SUCCEEDED) {
			// ------------------------------------------------------------------------------------
			// Semantic Analysis Phase ------------------------------------------------------------
			logDebugging(logger, "Performing semantic analysis...");
			compilationStatus = analyzeProgram(program);
			// ------------------------------------------------------------------------------------
			if (compilationStatus == SUCCEEDED) {
				// --------------------------------------------------------------------------------
				// Beginning of the Backend... ----------------------------------------------------
				logDebugging(logger, "Generating output...");
				executeGenerator(&compilerState);
				// ...end of the Backend. ---------------------------------------------------------
				// --------------------------------------------------------------------------------
			}
			else {
				logError(logger, "The semantic-analysis phase rejects the input program.");
				compilationStatus = FAILED;
			}
		}
	}
	else {
		logError(logger, "The syntactic-analysis phase rejects the input program.");
		compilationStatus = FAILED;
	}
	logDebugging(logger, "Releasing AST resources...");
	destroyProgram(program);
	for (int k = (sizeof(moduleDestructors)/sizeof(ModuleDestructor)) - 1; 0 <= k; --k) {
		moduleDestructors[k]();
	}
	logDebugging(logger, "Compilation is done.");
	destroyLogger(logger);
	destroyLexicalAnalyzer(lexicalAnalyzer);
	return compilationStatus;
}
