//
// Created by karol on 30.12.17.
//

#ifndef MISP_UTILS_H
#define MISP_UTILS_H

// Logging level
extern int LEVEL;
//int LEVEL = 0;

// Available levels
extern int DEBUG;
extern int ERROR;
extern int WARN;
extern int INFO;
extern int TRACE;

#define error(msg) (LEVEL >= ERROR && std::cerr << msg)
#define warn(msg) (LEVEL >= WARN && std::cerr << msg)
#define info(msg) (LEVEL >= INFO && std::cerr << msg)
#define debug(msg) (LEVEL >= DEBUG && std::cerr << msg)
#define trace(msg) (LEVEL >= TRACE && std::cerr << msg)

#endif //MISP_UTILS_H
