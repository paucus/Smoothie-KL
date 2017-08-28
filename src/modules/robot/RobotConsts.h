#ifndef ROBOT_CONSTS_H
#define ROBOT_CONSTS_H

#define Z_MAX_SPEED 1000
// TODO There's a way to make this expansion with C preprocessor, but I can't find how
#define Z_MAX_SPEED_STR "1000"
#define XY_MAX_SPEED 12000
#define E_MAX_SPEED 100
#define MILLISECONDS_PER_MINUTE 60000.0
// This is an upper bound of X
#define X_UPPER_BOUND_STR "378"
#define X_UPPER_BOUND 350
#define Y_UPPER_BOUND_STR "210"
#define Y_UPPER_BOUND 200
#define Z_UPPER_BOUND_STR "350"
#define Z_UPPER_BOUND 350


#endif // ROBOT_CONSTS_H
