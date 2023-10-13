//
// Created by Administrator on 2023/10/12.
//

/*
 * md5sum：计算md5校验和算法
 */

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "md5.h"

#define A 0x67452301
#define B 0xefcdab89
#define C 0x98badcfe
#define D 0x10325476

#define F(x, y ,z) ((x & y) | ((~x) & z))
#define G(x, y, z) ((x & z) | (y & (~z)))
#define H(x, y, z) (x ^ y ^ z)
#define I(x, y, z) (y ^ (x | (~z)))
#define rl_shift(x,n) ((x << n) | (x >> (32 - n)))

const uint32_t k[]={
        0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,
        0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,0x698098d8,
        0x8b44f7af,0xffff5bb1,0x895cd7be,0x6b901122,0xfd987193,
        0xa679438e,0x49b40821,0xf61e2562,0xc040b340,0x265e5a51,
        0xe9b6c7aa,0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
        0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,
        0xfcefa3f8,0x676f02d9,0x8d2a4c8a,0xfffa3942,0x8771f681,
        0x6d9d6122,0xfde5380c,0xa4beea44,0x4bdecfa9,0xf6bb4b60,
        0xbebfbc70,0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,
        0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,0xf4292244,
        0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,
        0xffeff47d,0x85845dd1,0x6fa87e4f,0xfe2ce6e0,0xa3014314,
        0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
};
//向左位移数
const uint8_t s[]={
        7,12,17,22,7,12,17,22,
        7,12,17,22,7,12,17,22,
        5,9,14,20,5,9,14,20,
        5,9,14,20,5,9,14,20,
        4,11,16,23,4,11,16,23,
        4,11,16,23,4,11,16,23,
        6,10,15,21,6,10,15,21,
        6,10,15,21,6,10,15,21
};


// 请确保msg的长度不需要填充
static uint8_t *padding(const char *msg, size_t *len)
{
    uint8_t *data;
    size_t msg_len = strlen(msg);
    size_t dlen;

    if (msg_len % 64 == 56) {
        dlen = msg_len + 8;
    } else {
        dlen = (msg_len + 63) & ~63;
    }
    if (!(data = (uint8_t*)malloc(dlen)))
        return NULL;
    *len = dlen;
    memset(data, 0, dlen);
    memcpy(data, msg, msg_len);
    if (msg_len % 64 != 56)
        data[msg_len] = 0x80;
    *(uint64_t *)(data + dlen - 8) = msg_len * 8;
    return data;
}

// 处理一级：16个整数（32位）
static void loop(uint32_t M[], int n, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d)
{
    int idx, i = 0;
    uint32_t f, g, at, bt, ct, dt;

    if (n != 16)
        return;

    at = *a;
    bt = *b;
    ct = *c;
    dt = *d;

    for (i = 0; i < 64; i++) {
        if (i < 16) {
            f = F(bt, ct, dt);
            idx = i;
        } else if (i < 32) {
            f = G(bt, ct, dt);
            idx = (5 * i + 1) % 16;
        } else if (i < 48) {
            f = H(bt, ct, dt);
            idx = (3 * i + 5) % 16;
        } else {
            f = I(bt, ct, dt);
            idx = (7 * i) % 16;
        }
        uint32_t tmp = dt;
        dt = ct;
        ct = bt;
        bt = bt + rl_shift((at + f + k[i] + M[idx]), s[i]);
        at = tmp;
    }
    *a += at;
    *b += bt;
    *c += ct;
    *d += dt;
}

static void int2str(uint32_t a, char *str, int len)
{
    int i;
    uint8_t b, t;
    char *p = str;

    if (len < 8)
        return;

    for (i = 0; i < 4; i++) {
        b = (a >> (i * 8)) & 0x0ff;
        t = (b >> 4) & 0x0f;
        *p++ = (t >= 10) ? t - 10 + 'A' : t + '0';
        t = b & 0x0f;
        *p++ = (t >= 10) ? t - 10 + 'A' : t + '0';
    }
}

int md5sum(const char *msg, char *md5_value, int len)
{
    size_t dlen, msg_len = strlen(msg);
    uint8_t *data = NULL;
    uint32_t a, b, c, d;
    int i, groups = 0;

    if (!msg || !md5_value || len < 33)
        return -1;
    memset(md5_value, 0, len);

    // 检查是否需要填充
    if (!(data = padding(msg, &dlen)))
        return -1;
    groups = dlen / 64;

    a = A;
    b = B;
    c = C;
    d = D;
    for (i = 0; i < groups; i++)
        loop((uint32_t *)(data + i * 16), 16, &a, &b, &c, &d);

    // 将a, b, c, d组合成字符串
    int2str(a, md5_value, 8);
    int2str(b, md5_value + 8, 8);
    int2str(c, md5_value + 16, 8);
    int2str(d, md5_value + 24, 8);
    LOG_INFO("md5sum: %s", md5_value);

    return 0;
}

