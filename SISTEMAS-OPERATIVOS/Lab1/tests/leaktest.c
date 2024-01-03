#include "test_scommand.h"
#include "test_pipeline.h"
/* #include "test_parser.h" */

int main (void)
{
	scommand_memory_test();
	pipeline_memory_test();
/*	parser_memory_test(); */
	return 0;
}

