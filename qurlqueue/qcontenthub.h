#ifndef QCONTENTHUB_H
#define QCONTENTHUB_H

#define DEFAULT_QUEUE_CAPACITY 1000

#define QCONTENTHUB_OK 0
#define QCONTENTHUB_WARN 2
#define QCONTENTHUB_ERROR -1

#define QCONTENTHUB_AGAIN 1
#define QCONTENTHUB_END 10

static const std::string QCONTENTHUB_DEFAULT_QUEUE = "";

static const std::string QCONTENTHUB_STRAGAIN  = "###again###";
static const std::string QCONTENTHUB_STRWARN = "###error###";
static const std::string QCONTENTHUB_STRERROR = "###error###";

static const std::string QCONTENTHUB_STREND = "###end###";

#endif
