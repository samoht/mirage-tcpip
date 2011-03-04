/*
 * Copyright (c) 2011 Julian Chesterfield <julian.chesterfield@citrix.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
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

#include "mirage-fs.h"
#include <arpa/inet.h>

#define htonll(x) \
((((x) & 0xff00000000000000LL) >> 56) | \
(((x) & 0x00ff000000000000LL) >> 40) | \
(((x) & 0x0000ff0000000000LL) >> 24) | \
(((x) & 0x000000ff00000000LL) >> 8) | \
(((x) & 0x00000000ff000000LL) << 8) | \
(((x) & 0x0000000000ff0000LL) << 24) | \
(((x) & 0x000000000000ff00LL) << 40) | \
(((x) & 0x00000000000000ffLL) << 56))

struct fs_hdr *init_hdr(char *filename, int length, u_long offset) {
  struct fs_hdr *fsh;
  fsh = malloc(sizeof(struct fs_hdr));
  fsh->magic = htons(MAGIC_HDR);
  fsh->offset = htonll(offset);
  fsh->length = htonll(length);
  fsh->namelen = htonl(strlen(filename));
  strncpy(fsh->filename, filename, 485);
  return fsh;
}

struct fs_hdr *read_hdr(int fd) {
  struct fs_hdr *fsh;

  fsh = malloc(sizeof(struct fs_hdr));

  //only handle sector reads
  if (read(fd, fsh, SECTOR_SIZE)!=SECTOR_SIZE)
    goto exit;
  if(fsh->magic != MAGIC_HDR)
    goto exit;

  return fsh;
 exit:
  free(fsh);
  return NULL;
}

int fcopy(int infd, int outfd, u_long length) {
  char buf[SECTOR_SIZE], *p;
  int inbytes, outbytes, count=0;

  while(length > 0) {
    bzero(buf,SECTOR_SIZE);
    inbytes = read(infd, buf, SECTOR_SIZE);
    if(inbytes < 1)
      return count;
    
    length -= SECTOR_SIZE;
    outbytes = write(outfd, buf, SECTOR_SIZE);
    if(outbytes != SECTOR_SIZE)
	return count;
    count += outbytes;
  }
  return count;  
}

int fzero(int outfd, int length) {
  //Length must be a multiple of page size
  char *buf;
  int i;

  buf = calloc(1,4096);
  for(i=0; i<(length/4096); i++)
    write(outfd, buf, 4096);
  free(buf);
  return 1;
}
