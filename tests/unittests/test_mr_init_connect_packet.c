#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "mister/mister.h"

/* A test case that does nothing and succeeds. */
//static void null_test_success(void **state) {
//    (void) state; /* unused */
//}

static void test_mr_init_connect_packet(void **state) {
    packet_ctx *pctx;
    int rc = mr_init_connect_packet(&pctx);
    assert_return_code(rc, 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        //cmocka_unit_test(null_test_success),
        cmocka_unit_test(test_mr_init_connect_packet),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
