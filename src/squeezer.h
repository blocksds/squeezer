// SPDX-License-Identifier: ISC

/* Copyright (c) huxingyi@msn.com All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef SQUEEZER_H
#define SQUEEZER_H

typedef struct squeezer squeezer;

squeezer *squeezerCreate(void);
void squeezerSetBinWidth(squeezer *ctx, int width);
void squeezerSetBinHeight(squeezer *ctx, int height);
void squeezerSetAllowRotations(squeezer *ctx, int allowRotations);
void squeezerSetVerbose(squeezer *ctx, int verbose);
void squeezerSetHasBorder(squeezer *ctx, int hasBorder);
int squeezerDoDir(squeezer *ctx, const char *dir);
void squeezerDestroy(squeezer *ctx);
int squeezerOutputImage(squeezer *ctx, const char *filename);
int squeezerOutputXml(squeezer *ctx, const char *filename);
int squeezerOutputC(squeezer *ctx, const char *outputBaseName, const char *outputCFilename,
  const char *outputHFilename, const char *outputNitroFilename);
int squeezerOutputCustomFormat(squeezer *ctx, const char *filename,
  const char *header, const char *body, const char *footer, const char *split);

#endif
