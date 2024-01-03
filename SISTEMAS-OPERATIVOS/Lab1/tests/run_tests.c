#include <check.h>
#include <stdlib.h> /* Para tener EXIT_XXX */

#ifdef TEST_COMMAND
#include "test_scommand.h"
#include "test_pipeline.h"
#endif /* TEST_COMMAND */

#ifdef TEST_PARSER
#include "test_parser.h"
#endif /* TEST_PARSER */

#ifdef TEST_EXECUTE
#include "test_execute.h"
#endif /* TEST_EXECUTE */

int main (void)
{
    int number_failed;
    SRunner *sr = srunner_create(NULL);

#ifdef TEST_COMMAND
    srunner_add_suite(sr, scommand_suite());
    srunner_add_suite(sr, pipeline_suite());
#endif /* TEST_COMMAND */

#ifdef TEST_PARSER
    srunner_add_suite(sr, parser_suite());
#endif /* TEST_PARSER */

#ifdef TEST_EXECUTE
    srunner_add_suite(sr, execute_suite());
#endif /* TEST_EXECUTE */

    srunner_set_log(sr, "test.log");
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
