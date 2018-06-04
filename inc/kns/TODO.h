#pragma once

#define align_up(x, a)	((((x) - 1) | ((typeof(x)) (a) - 1)) + 1)