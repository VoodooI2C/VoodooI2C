//
//  linuxirq.hpp
//  VoodooGPIO
//
//  Created by CoolStar on 8/14/17.
//  Copyright © 2017 CoolStar. All rights reserved.
//

#ifndef linuxirq_hpp
#define linuxirq_hpp

enum {
    IRQ_TYPE_NONE		= 0x00000000,
    IRQ_TYPE_EDGE_RISING	= 0x00000001,
    IRQ_TYPE_EDGE_FALLING	= 0x00000002,
    IRQ_TYPE_EDGE_BOTH	= (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING),
    IRQ_TYPE_LEVEL_HIGH	= 0x00000004,
    IRQ_TYPE_LEVEL_LOW	= 0x00000008,
    IRQ_TYPE_LEVEL_MASK	= (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH),
    IRQ_TYPE_SENSE_MASK	= 0x0000000f,
    IRQ_TYPE_DEFAULT	= IRQ_TYPE_SENSE_MASK,
    
    IRQ_TYPE_PROBE		= 0x00000010,
    
    IRQ_LEVEL		= (1 <<  8),
    IRQ_PER_CPU		= (1 <<  9),
    IRQ_NOPROBE		= (1 << 10),
    IRQ_NOREQUEST		= (1 << 11),
    IRQ_NOAUTOEN		= (1 << 12),
    IRQ_NO_BALANCING	= (1 << 13),
    IRQ_MOVE_PCNTXT		= (1 << 14),
    IRQ_NESTED_THREAD	= (1 << 15),
    IRQ_NOTHREAD		= (1 << 16),
    IRQ_PER_CPU_DEVID	= (1 << 17),
    IRQ_IS_POLLED		= (1 << 18),
    IRQ_DISABLE_UNLAZY	= (1 << 19),
};

#endif /* linuxirq_hpp */