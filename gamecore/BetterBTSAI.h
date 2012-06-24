#pragma once

#ifndef BETTERBTSAI_H
#define BETTERBTSAI_H

// AI decision making logging

// Log levels:
// 0 - None
// 1 - Important decisions only
// 2 - Many decisions
// 3 - All logging
#ifdef LOG_AI
#define gPlayerLogLevel 3
#define gTeamLogLevel   3
#define gCityLogLevel   3
#define gUnitLogLevel   3
#else
#define gPlayerLogLevel 0
#define gTeamLogLevel   0
#define gCityLogLevel   0
#define gUnitLogLevel   0
#endif

void logBBAI(char* format, ... );

#endif  //BETTERBTSAI_H