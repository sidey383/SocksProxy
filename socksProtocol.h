#pragma once
#pragma pack(1)

#include "stdint.h"

#define OK_STATUS 1
#define ERROR_STATUS 0

typedef enum {
    NoAuth = 0x00,
    GSSAPI = 0x01,
    UserPassword = 0x02,
    NoMethod = 0xFF
} authMethod;

typedef enum {
    OK = 0x00,
    InternalError = 0x01,
    NotAllowed = 0x02,
    NetworkUnreachable = 0x03,
    HostUnreachable = 0x04,
    ConnectionRefused = 0x05,
    TTLExpired = 0x06,
    ProtocolError = 0x07,
    AddressTypeNotSupported = 0x08
} answerCode;

typedef struct {
    uint8_t count;
    uint8_t *methods;
} authMethods;

typedef struct {
    uint8_t version;
    uint8_t authMethod;
} authAnswer;

typedef struct {
    uint8_t version;
    uint8_t command;
    /* must be 0x00 */
    uint8_t reserved;
    uint8_t addressType;
    unsigned char address[4];
    uint16_t portBE;
} clientRequestIPv4;

typedef struct {
    uint8_t version;
    uint8_t command;
    /* must be 0x00 */
    uint8_t reserved;
    uint8_t addressType;
    unsigned char address[16];
    uint16_t portBE;
} clientRequestIPv6;

typedef struct {
    uint8_t version;
    uint8_t command;
    /* must be 0x00 */
    uint8_t reserved;
    uint8_t addressType;
    unsigned char *address;
    uint16_t portBE;
} clientRequestDomain;


typedef struct {
    uint8_t version;
    uint8_t command;
    /* must be 0x00 */
    uint8_t reserved;
    uint8_t addressType;
} baseClientRequest;

typedef union {
    baseClientRequest base;
    clientRequestIPv4 ipv4;
    clientRequestIPv6 ipv6;
    clientRequestDomain domain;
} clientRequest;

typedef struct {
    uint8_t version;
    uint8_t answer;
    /* must be 0x00 */
    uint8_t reserved;
    uint8_t addressType;
    unsigned char address[4];
    uint16_t portBE;
} serverAnswerIPv4;

typedef struct {
    uint8_t version;
    uint8_t answer;
    /* must be 0x00 */
    uint8_t reserved;
    uint8_t addressType;
    unsigned char address[16];
    uint16_t portBE;
} serverAnswerIPv6;

typedef struct {
    uint8_t version;
    uint8_t answer;
    /* must be 0x00 */
    uint8_t reserved;
    uint8_t addressType;
    unsigned char *address;
    uint16_t portBE;
} serverAnswerDomain;


typedef struct {
    uint8_t version;
    uint8_t answer;
    /* must be 0x00 */
    uint8_t reserved;
    uint8_t addressType;
} baseServerAnswer;


typedef struct {
    baseServerAnswer base;
    serverAnswerIPv4 ipv4;
    serverAnswerIPv6 iPv6;
    serverAnswerDomain domain;
} serverAnswer;

authMethods* acceptAuth(int fd);

void authMethods_deconstruct(authMethods* r);

authAnswer* createAuthAnswer(authMethod method);

void sendAuthAnswer(int fd, authAnswer* answer);

void authAnswer_deconstruct(authAnswer* a);

clientRequest* acceptRequest(int fd);

void clientRequest_deconstruct(clientRequest* r);


