#ifndef MATH_OPERATIONS_H
#define MATH_OPERATIONS_H

/* Simple math operations header.
 * Provides an add function that returns the sum of two ints.
 *
 * This header defines add as a static inline function to allow
 * inclusion in multiple translation units without linker errors.
 */

#ifdef __cplusplus
extern "C" {
#endif

static inline int add(int a, int b) {
    return a + b;
}

#ifdef __cplusplus
}
#endif

#endif /* MATH_OPERATIONS_H */
