#ifndef YAML_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define YAML_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include "yaml/parser.h"
#include "yaml/emitter.h"
#include "yaml/emitterstyle.h"
#include "yaml/stlemitter.h"
#include "yaml/exceptions.h"

#include "yaml/node/node.h"
#include "yaml/node/impl.h"
#include "yaml/node/convert.h"
#include "yaml/node/iterator.h"
#include "yaml/node/detail/impl.h"
#include "yaml/node/parse.h"
#include "yaml/node/emit.h"

#endif  // YAML_H_62B23520_7C8E_11DE_8A39_0800200C9A66
