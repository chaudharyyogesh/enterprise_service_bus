// gcc test_esb.c  bmd.c ./transport/send_email.c ./test/munit.c `xml2-config --cflags --libs` `mysql_config --cflags --libs` -lcurl -lxml2 -o test_esb.o
// ./test_esb.o

#include <stdio.h>
#include "./test/munit.h"
#include "bmd.h"
#include "./transport/send_email.h"

static void * test_xml_values_setup(const MunitParameter params[], void *user_data)
{
    /**
     * Return the data that will be used for test_xml_values. Here we
     * are just return a string. It can be a struct or anything.
     * The memory that you allocate here for the test data
     * has to be cleaned up in corresponding tear down function,
     * which in this case is test_tear_down.
     */
    return strdup("/home/yogesh/Downloads/nho2021/Goat/received_bmd.xml");
}

static void test_xml_values_tear_down(void *fixture)
{
    /* Receives the pointer to the data if that that was created in
    test_setup function. */
    free(fixture);
}

// For testing the bmd values
static MunitResult test_xml_values(const MunitParameter params[], void* fixture) {
  char *path = (char *)fixture;

 


  BMD *test_bmd= parse_bmd_file(path);

  printf("%s\n" ,test_bmd->envelop.message_id);

  if(strcmp(path,"/home/yogesh/Downloads/nho2021/Goat/received_bmd.xml")==0) { 
    //compare each and every data of xml with the expected data
    munit_assert_string_equal(test_bmd->envelop.message_id,"SSSM45AF2-107A-4452-9553-043B6D4521SW");
    munit_assert_string_equal(test_bmd->envelop.message_type,"CreditReport");
    munit_assert_string_equal(test_bmd->envelop.sender_id,"AED2EAA-1D5B-4BC0-ACC4-4S45454LMK");
    munit_assert_string_equal(test_bmd->envelop.destination_id,"5SA3F82F-4547-4331-DD23-13F3A4441FJ");
    munit_assert_string_equal(test_bmd->envelop.creation_time,"2018-01-11T07:30:22+0000");
    munit_assert_string_equal(test_bmd->envelop.reference_id,"INV-PROFILE-884999");
    munit_assert_string_equal(test_bmd->envelop.signature,"41526Ssdf9301f715433f8f3689390d1f5da5");
    munit_assert_string_equal(test_bmd->payload,"Hi there. Payload.");

    }

  return MUNIT_OK;
}

//For testing the validation of the BMD
static MunitResult test_bmd_valid(const MunitParameter params[], void* fixture) {
  //char *path = (char *)fixture;

  BMD *test_bmd= parse_bmd_file("/home/yogesh/Downloads/nho2021/Goat/received_bmd.xml");

  printf("%s\n" ,test_bmd->envelop.message_id);
 
  //validation test
  int valid =is_bmd_valid(test_bmd);
  munit_assert_int(valid,==,1);

  return MUNIT_OK;
}

//For testing the email service
static MunitResult
test_email_service(const MunitParameter params[], void * fixture) {
    int status = send_email("chaudharyyogesh9818743347@gmail.com", "/home/yogesh/Downloads/nho2021/Goat/received_bmd.xml");
    munit_assert_int(status, == , 0);
    return MUNIT_OK;
}

static MunitTest esb_tests[] = {
  
  
  { (char*) "/test_xml_values", test_xml_values, test_xml_values_setup , test_xml_values_tear_down, MUNIT_TEST_OPTION_NONE, NULL},
   
   { (char*) "/test_bmd_valid", test_bmd_valid, NULL , NULL, MUNIT_TEST_OPTION_NONE, NULL},

//   { (char*) "/test_queue_request",test_queue_request, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

  { (char*) "/test_email_service",test_email_service, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },


//  { (char*) "/test_ftp",test_ftp, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

//  { (char*) "/test_http",test_http, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
 
//  { (char*) "/test_transform",test_transform, NULL,NULL,MUNIT_TEST_OPTION_NONE, NULL },
 
//  { (char*) "/test_CSVtransform",test_CSVtransform, NULL,NULL,MUNIT_TEST_OPTION_NONE, NULL },
 
 
  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};



/* Now we'll actually declare the test suite.  You could do this in
 * the main function, or on the heap, or whatever you want. */
static const MunitSuite test_suite = {
  /* This string will be prepended to all test names in this suite;
   * for example, "/example/rand" will become "/µnit/example/rand".
   * Note that, while it doesn't really matter for the top-level
   * suite, NULL signal the end of an array of tests; you should use
   * an empty string ("") instead. */
  (char*) "",
  /* The first parameter is the array of test suites. */
  esb_tests,
  /* In addition to containing test cases, suites can contain other
   * test suites.  This isn't necessary in this example, but it can be
   * a great help to projects with lots of tests by making it easier
   * to spread the tests across many files.  This is where you would
   * put "other_suites" (which is commented out above). */
  NULL,
  /* An interesting feature of µnit is that it supports automatically
   * running multiple iterations of the tests.  This is usually only
   * interesting if you make use of the PRNG to randomize your tests
   * cases a bit, or if you are doing performance testing and want to
   * average multiple runs.  0 is an alias for 1. */
  1,
  /* Just like MUNIT_TEST_OPTION_NONE, you can provide
   * MUNIT_SUITE_OPTION_NONE or 0 to use the default settings. */
  MUNIT_SUITE_OPTION_NONE
};

/* This is only necessary for EXIT_SUCCESS and EXIT_FAILURE, which you
 * *should* be using but probably aren't (no, zero and non-zero don't
 * always mean success and failure).  I guess my point is that nothing
 * about µnit requires it. */
#include <stdlib.h>
#if 1
int main(int argc, char* argv[]) {
  /* Finally, we'll actually run our test suite!  That second argument
   * is the user_data parameter which will be passed either to the
   * test or (if provided) the fixture setup function. */
  return munit_suite_main(&test_suite, NULL, argc, NULL );
}
#endif
