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

#include "squeezer.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SQUEEZERW_VER VERSION_STRING

static const char *dir = 0;
static int binWidth = 0;
static int binHeight = 0;
static int allowRotations = 0;
static int verbose = 0;
static const char *outputTextureFilename = "squeezer.png";
static const char *outputInfoFilename = "squeezer.xml";
static const char *outputHFilename = NULL;
static const char *outputCFilename = NULL;
static const char *outputNitroFilename = NULL;
static const char *outputBaseName = NULL;
static const char *infoHeader = 0;
static const char *infoFooter = 0;
static const char *infoBody = 0;
static const char *infoSplit = 0;
static int border = 0;

static void usage(void) {
  fprintf(stderr, "squeezerw " SQUEEZERW_VER "\n"
    "usage: squeezerw [options] <sprite image dir>\n"
    "    options:\n"
    "        --width <output width>\n"
    "        --height <output height>\n"
    "        --allowRotations <1/0/true/false/yes/no>\n"
    "        --border <1/0/true/false/yes/no>\n"
    "        --outputTexture <output texture filename>\n"
    "        --outputInfo <output sprite info filename>\n"
    "        --outputH <output sprite info header file>\n"
    "        --outputC <output sprite info C file>\n"
    "        --outputNitro <output sprite info NitroFS file>\n"
    "        --outputBaseName <output C base name>\n"
    "        --infoHeader <output header template>\n"
    "        --infoBody <output body template>\n"
    "        --infoSplit <output body split template>\n"
    "        --infoFooter <output footer template>\n"
    "        --verbose\n"
    "        --version\n"
    "    format specifiers of infoHeader/infoBody/infoFooter:\n"
    "        %%W: output width\n"
    "        %%H: output height\n"
    "        %%n: image name\n"
    "        %%w: image width\n"
    "        %%h: image height\n"
    "        %%x: left on output image\n"
    "        %%y: top on output image\n"
    "        %%l: trim offset left\n"
    "        %%t: trim offset top\n"
    "        %%c: original width\n"
    "        %%r: original height\n"
    "        %%f: 1 if rotated else 0\n"
    "        and '\\n', '\\r', '\\t'\n");
}

static int parseBooleanParam(const char *param) {
  switch (param[0]) {
    case 'F':
    case 'f':
    case 'N':
    case 'n':
    case '0':
      return 0;
  }
  return 1;
}

static int squeezerw(void) {
  squeezer *ctx = squeezerCreate();
  if (!ctx) {
    fprintf(stderr, "%s: squeezerCreate failed\n", __FUNCTION__);
    return -1;
  }
  squeezerSetAllowRotations(ctx, allowRotations);
  squeezerSetVerbose(ctx, verbose);
  squeezerSetHasBorder(ctx, border);

  if ((binWidth != 0) && (binHeight != 0)) {
    // If the size is provided by the user, try only with that size
    squeezerSetBinWidth(ctx, binWidth);
    squeezerSetBinHeight(ctx, binHeight);

    if (verbose)
      printf("Trying with %dx%d\n", binWidth, binHeight);

    if (0 != squeezerDoDir(ctx, dir)) {
      fprintf(stderr, "%s: squeezerDoDir failed\n", __FUNCTION__);
      squeezerDestroy(ctx);
      return -1;
    }
  } else {
    // If the size hasn't been specified, retry all possible sizes
    bool retryWidth = false;
    bool retryHeight = false;

    if (binWidth == 0) {
        binWidth = 8;
        retryWidth = true;
    }
    if (binHeight == 0) {
        binHeight = 8;
        retryHeight = true;
    }

    while (1) {
      squeezerSetBinWidth(ctx, binWidth);
      squeezerSetBinHeight(ctx, binHeight);

      if (verbose)
        printf("Trying with %dx%d\n", binWidth, binHeight);

      if (0 == squeezerDoDir(ctx, dir))
        break;

      if (retryWidth && retryHeight) {
        if (binWidth < binHeight)
          binWidth *=2;
        else
          binHeight *=2;
      } else if (retryWidth) {
        binWidth *=2;
      } else if (retryHeight) {
        binHeight *=2;
      }

      if (binWidth > 1024 || binHeight > 1024) {
        fprintf(stderr, "%s: squeezerDoDir failed for all allowed sizes for the DS\n", __FUNCTION__);
        squeezerDestroy(ctx);
        return -1;
      }
    }
  }

  if (0 != squeezerOutputImage(ctx, outputTextureFilename)) {
    fprintf(stderr, "%s: squeezerOutputImage failed\n", __FUNCTION__);
    squeezerDestroy(ctx);
    return -1;
  }

  if (outputBaseName || outputNitroFilename) {
    if (0 != squeezerOutputC(ctx, outputBaseName, outputCFilename, outputHFilename,
        outputNitroFilename)) {
      fprintf(stderr, "%s: squeezerOutputC failed\n", __FUNCTION__);
      squeezerDestroy(ctx);
      return -1;
    }
  } else if (infoBody) {
    if (0 != squeezerOutputCustomFormat(ctx, outputInfoFilename,
        infoHeader, infoBody, infoFooter, infoSplit)) {
      fprintf(stderr, "%s: squeezerOutputCustomFormat failed\n", __FUNCTION__);
      squeezerDestroy(ctx);
      return -1;
    }
  } else {
    if (0 != squeezerOutputXml(ctx, outputInfoFilename)) {
      fprintf(stderr, "%s: squeezerOutputXml failed\n", __FUNCTION__);
      squeezerDestroy(ctx);
      return -1;
    }
  }
  squeezerDestroy(ctx);
  return 0;
}

int main(int argc, char *argv[]) {
  int i;
  if (argc < 2) {
    usage();
    return -1;
  }
  for (i = 1; i < argc; ++i) {
    const char *param = argv[i];
    if ('-' == param[0]) {
      if (0 == strcmp(param, "--version")) {
        printf("squeezerw " SQUEEZERW_VER "\n");
        return 0;
      } else if (0 == strcmp(param, "--width")) {
        binWidth = atoi(argv[++i]);
      } else if (0 == strcmp(param, "--height")) {
        binHeight = atoi(argv[++i]);
      } else if (0 == strcmp(param, "--allowRotations")) {
        allowRotations = parseBooleanParam(argv[++i]);
      } else if (0 == strcmp(param, "--border")) {
        border = parseBooleanParam(argv[++i]);
      } else if (0 == strcmp(param, "--outputTexture")) {
        outputTextureFilename = argv[++i];
      } else if (0 == strcmp(param, "--outputInfo")) {
        outputInfoFilename = argv[++i];
      } else if (0 == strcmp(param, "--outputH")) {
        outputHFilename = argv[++i];
      } else if (0 == strcmp(param, "--outputC")) {
        outputCFilename = argv[++i];
      } else if (0 == strcmp(param, "--outputNitro")) {
        outputNitroFilename = argv[++i];
      } else if (0 == strcmp(param, "--outputBaseName")) {
        outputBaseName = argv[++i];
      } else if (0 == strcmp(param, "--infoHeader")) {
        infoHeader = argv[++i];
      } else if (0 == strcmp(param, "--infoBody")) {
        infoBody = argv[++i];
      } else if (0 == strcmp(param, "--infoSplit")) {
        infoSplit = argv[++i];
      } else if (0 == strcmp(param, "--infoFooter")) {
        infoFooter = argv[++i];
      } else if (0 == strcmp(param, "--verbose")) {
        verbose = 1;
      } else {
        usage();
        fprintf(stderr, "%s: unknown arg: %s\n", __FUNCTION__, param);
        return -1;
      }
    } else {
      dir = param;
    }
  }
  if (!dir) {
    usage();
    return -1;
  }
  if (verbose) {
    printf("squeezering using the follow options:\n"
      "    --width %d\n"
      "    --height %d\n"
      "    --allowRotations %s\n"
      "    --border %s\n"
      "    --outputTexture %s\n"
      "    --outputInfo %s\n"
      "    --outputH %s\n"
      "    --outputC %s\n"
      "    --outputNitro %s\n"
      "    --outputBaseName %s\n"
      "    --infoHeader %s\n"
      "    --infoBody %s\n"
      "    --infoFooter %s\n"
      "    --infoSplit %s\n"
      "%s",
      binWidth,
      binHeight,
      allowRotations ? "true" : "false",
      border ? "true" : "false",
      outputTextureFilename,
      outputInfoFilename,
      outputHFilename ? outputHFilename : "",
      outputCFilename ? outputCFilename : "",
      outputNitroFilename ? outputNitroFilename : "",
      outputBaseName ? outputBaseName : "",
      infoHeader ? infoHeader : "",
      infoBody ? infoBody : "",
      infoFooter ? infoFooter : "",
      infoSplit ? infoSplit : "",
      verbose ? "    --verbose\n" : "");
  }
  return squeezerw();
}
