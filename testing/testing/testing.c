//
//  testing.c
//  testing
//
//  Created by Alexandre on 17/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include <mach/mach_types.h>

kern_return_t testing_start(kmod_info_t * ki, void *d);
kern_return_t testing_stop(kmod_info_t *ki, void *d);

kern_return_t testing_start(kmod_info_t * ki, void *d)
{
    return KERN_SUCCESS;
}

kern_return_t testing_stop(kmod_info_t *ki, void *d)
{
    return KERN_SUCCESS;
}
