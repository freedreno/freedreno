/*
 * Copyright © 2012 Rob Clark <robclark@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* bits and pieces borrowed from lima project.. concept is the same, wrap
 * various syscalls and log what happens
 */

#include <ctype.h>

#include "wrap.h"

#ifdef USE_PTHREADS
static pthread_mutex_t l = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;
#define LOCK()   pthread_mutex_lock(&l)
#define UNLOCK() pthread_mutex_unlock(&l)
#else
#define LOCK()
#define UNLOCK()
#endif

struct device_info {
	const char *name;
	struct {
		const char *name;
	} ioctl_info[_IOC_NR(0xffffffff)];
};

#define IOCTL_INFO(n) \
		[_IOC_NR(n)] = { .name = #n }

static struct device_info kgsl_3d_info = {
		.name = "kgsl-3d",
		.ioctl_info = {
				IOCTL_INFO(IOCTL_KGSL_DEVICE_GETPROPERTY),
				IOCTL_INFO(IOCTL_KGSL_DEVICE_WAITTIMESTAMP),
				IOCTL_INFO(IOCTL_KGSL_DEVICE_WAITTIMESTAMP_CTXTID),
				IOCTL_INFO(IOCTL_KGSL_RINGBUFFER_ISSUEIBCMDS),
				IOCTL_INFO(IOCTL_KGSL_CMDSTREAM_READTIMESTAMP),
				IOCTL_INFO(IOCTL_KGSL_CMDSTREAM_FREEMEMONTIMESTAMP),
				IOCTL_INFO(IOCTL_KGSL_DRAWCTXT_CREATE),
				IOCTL_INFO(IOCTL_KGSL_DRAWCTXT_DESTROY),
				IOCTL_INFO(IOCTL_KGSL_MAP_USER_MEM),
				IOCTL_INFO(IOCTL_KGSL_SHAREDMEM_FROM_PMEM),
				IOCTL_INFO(IOCTL_KGSL_SHAREDMEM_FREE),
				IOCTL_INFO(IOCTL_KGSL_SHAREDMEM_FROM_VMALLOC),
				IOCTL_INFO(IOCTL_KGSL_SHAREDMEM_FLUSH_CACHE),
				IOCTL_INFO(IOCTL_KGSL_GPUMEM_ALLOC),
				IOCTL_INFO(IOCTL_KGSL_CFF_SYNCMEM),
				IOCTL_INFO(IOCTL_KGSL_CFF_USER_EVENT),
				IOCTL_INFO(IOCTL_KGSL_TIMESTAMP_EVENT),
				IOCTL_INFO(IOCTL_KGSL_GPUMEM_ALLOC_ID),
				IOCTL_INFO(IOCTL_KGSL_GPUMEM_FREE_ID),
				IOCTL_INFO(IOCTL_KGSL_PERFCOUNTER_GET),
				IOCTL_INFO(IOCTL_KGSL_PERFCOUNTER_PUT),
				/* kgsl-3d specific ioctls: */
				IOCTL_INFO(IOCTL_KGSL_DRAWCTXT_SET_BIN_BASE_OFFSET),
				IOCTL_INFO(IOCTL_KGSL_SUBMIT_COMMANDS),
				IOCTL_INFO(IOCTL_KGSL_SYNCSOURCE_CREATE),
				IOCTL_INFO(IOCTL_KGSL_SYNCSOURCE_DESTROY),
				IOCTL_INFO(IOCTL_KGSL_GPUOBJ_ALLOC),
				IOCTL_INFO(IOCTL_KGSL_GPUOBJ_FREE),
				IOCTL_INFO(IOCTL_KGSL_GPUOBJ_INFO),
				IOCTL_INFO(IOCTL_KGSL_GPU_COMMAND),
		},
};

// kgsl-2d => Z180 vector graphcis core.. not sure if it is interesting..
static struct device_info kgsl_2d_info = {
		.name = "kgsl-2d",
		.ioctl_info = {
				IOCTL_INFO(IOCTL_KGSL_DEVICE_GETPROPERTY),
				IOCTL_INFO(IOCTL_KGSL_DEVICE_WAITTIMESTAMP),
				IOCTL_INFO(IOCTL_KGSL_DEVICE_WAITTIMESTAMP_CTXTID),
				IOCTL_INFO(IOCTL_KGSL_RINGBUFFER_ISSUEIBCMDS),
				IOCTL_INFO(IOCTL_KGSL_CMDSTREAM_READTIMESTAMP),
				IOCTL_INFO(IOCTL_KGSL_CMDSTREAM_FREEMEMONTIMESTAMP),
				IOCTL_INFO(IOCTL_KGSL_DRAWCTXT_CREATE),
				IOCTL_INFO(IOCTL_KGSL_DRAWCTXT_DESTROY),
				IOCTL_INFO(IOCTL_KGSL_MAP_USER_MEM),
				IOCTL_INFO(IOCTL_KGSL_SHAREDMEM_FROM_PMEM),
				IOCTL_INFO(IOCTL_KGSL_SHAREDMEM_FREE),
				IOCTL_INFO(IOCTL_KGSL_SHAREDMEM_FROM_VMALLOC),
				IOCTL_INFO(IOCTL_KGSL_SHAREDMEM_FLUSH_CACHE),
				IOCTL_INFO(IOCTL_KGSL_GPUMEM_ALLOC),
				IOCTL_INFO(IOCTL_KGSL_CFF_SYNCMEM),
				IOCTL_INFO(IOCTL_KGSL_CFF_USER_EVENT),
				IOCTL_INFO(IOCTL_KGSL_TIMESTAMP_EVENT),
				IOCTL_INFO(IOCTL_KGSL_GPUMEM_ALLOC_ID),
				IOCTL_INFO(IOCTL_KGSL_GPUMEM_FREE_ID),
				/* no kgsl-2d specific ioctls, I don't think.. */
		},
};

static struct {
	int is_3d, is_2d;
#ifdef FAKE
	int is_emulated;
#endif
} file_table[1024];

static struct device_info * get_kgsl_info(int fd)
{
	if (fd >= ARRAY_SIZE(file_table))
		return NULL;
	if (file_table[fd].is_2d)
		return &kgsl_2d_info;
	else if (file_table[fd].is_3d)
		return &kgsl_3d_info;
	return NULL;
}

static void
hexdump(const void *data, int size)
{
	unsigned char *buf = (void *) data;
	char alpha[17];
	int i;

	for (i = 0; i < size; i++) {
		if (!(i % 16))
			printf("\t\t\t%08X", (unsigned int) i);
		if (!(i % 4))
			printf(" ");

		if (((void *) (buf + i)) < ((void *) data)) {
			printf("   ");
			alpha[i % 16] = '.';
		} else {
			printf(" %02x", buf[i]);

			if (isprint(buf[i]) && (buf[i] < 0xA0))
				alpha[i % 16] = buf[i];
			else
				alpha[i % 16] = '.';
		}

		if ((i % 16) == 15) {
			alpha[16] = 0;
			printf("\t|%s|\n", alpha);
		}
	}

	if (i % 16) {
		for (i %= 16; i < 16; i++) {
			printf("   ");
			alpha[i] = '.';

			if (i == 15) {
				alpha[16] = 0;
				printf("\t|%s|\n", alpha);
			}
		}
	}
}


static void
hexdump_dwords(const void *data, int sizedwords)
{
	uint32_t *buf = (void *) data;
	int i;

	for (i = 0; i < sizedwords; i++) {
		if (!(i % 8))
			printf("\t\t\t%08X:   ", (unsigned int) i*4);
		printf(" %08x", buf[i]);
		if ((i % 8) == 7)
			printf("\n");
	}

	if (i % 8)
		printf("\n");
}


static void dump_ioctl(struct device_info *info, int dir, int fd,
		unsigned long int request, void *ptr, int ret)
{
	int nr = _IOC_NR(request);
	int sz = _IOC_SIZE(request);
	char c;
	const char *name;

	if (dir == _IOC_READ)
		c = '<';
	else
		c = '>';

	if (info->ioctl_info[nr].name)
		name = info->ioctl_info[nr].name;
	else
		name = "<unknown>";

	printf("%c [%4d] %8s: %s (%08lx)", c, fd, info->name, name, request);
	if (dir == _IOC_READ)
		printf(" => %d", ret);
	printf("\n");

	if (dir & _IOC_DIR(request))
		hexdump(ptr, sz);
}

static void dumpfile(const char *file)
{
	char buf[1024];
	int fd = open(file, 0);
	int n;

	while ((n = read(fd, buf, 1024)) > 0)
		write(1, buf, n);
	close(fd);
}

struct buffer {
	void *hostptr;
	unsigned int len, handle, id;
	uint64_t flags;
	uint64_t gpuaddr;
	uint64_t offset;
	struct list node;
	int munmap;
	int dumped;
};

static LIST_HEAD(buffers_of_interest);

static struct buffer * register_buffer(void *hostptr, uint64_t flags,
		unsigned int len, unsigned int handle)
{
	struct buffer *buf = calloc(1, sizeof *buf);
	buf->hostptr = hostptr;
	buf->flags = flags;
	buf->len = len;
	buf->handle = handle;
	list_add(&buf->node, &buffers_of_interest);
	return buf;
}

static struct buffer * find_buffer(void *hostptr, uint64_t gpuaddr,
		uint64_t offset, unsigned int handle, unsigned id)
{
	struct buffer *buf = NULL;
	list_for_each_entry(buf, &buffers_of_interest, node) {
		if (hostptr)
			if ((buf->hostptr <= hostptr) && (hostptr < (buf->hostptr + buf->len)))
				return buf;
		if (gpuaddr)
			if ((buf->gpuaddr <= gpuaddr) && (gpuaddr < (buf->gpuaddr + buf->len)))
				return buf;
		if (offset)
			if ((buf->offset <= offset) && (offset < (buf->offset + buf->len)))
				return buf;
		if (handle)
			if (buf->handle == handle)
				return buf;
		if (id)
			if (buf->id == id)
				return buf;
	}
	return NULL;
}

static void unregister_buffer(struct buffer *buf)
{
	if (buf) {
		list_del(&buf->node);
		if (buf->munmap)
			munmap(buf->hostptr, buf->len);
		free(buf);
	}
}

static void dump_buffer(uint64_t gpuaddr)
{
	static int cnt = 0;
	struct buffer *buf = find_buffer((void *)-1, gpuaddr, 0, 0, 0);
	if (buf) {
		char filename[32];
		int fd;
		sprintf(filename, "%04d-%016lx.dat", cnt, buf->gpuaddr);
		printf("\t\tdumping: %s\n", filename);
		fd = open(filename, O_WRONLY| O_TRUNC | O_CREAT, 0644);
		write(fd, buf->hostptr, buf->len);
		close(fd);
		cnt++;
	}
}

#ifdef FAKE
static int is64b = 0;
uint64_t alloc_gpuaddr(uint32_t size)
{
	// TODO need better scheme to deal w/ deallocation..
	static uint32_t gpuaddr = 0xc0000000;
	uint32_t addr = gpuaddr;
	gpuaddr += size;
	if (is64b)
		return ((uint64_t)0x1ffff << 32) | addr;
	return addr;
}
#endif

/*****************************************************************************/
static void install_fd(const char *path, int fd)
{
	assert(fd < ARRAY_SIZE(file_table));
	if (!strcmp(path, "/dev/kgsl-3d0")) {
#ifdef FAKE
		assert(wrap_gpu_id() && wrap_gmem_size());
		if ((wrap_gpu_id() >= 500) && !env2u("WRAP_FORCE_32B"))
			is64b = 1;
		file_table[fd].is_emulated = 1;
#endif
		file_table[fd].is_3d = 1;
		printf("found kgsl_3d0: %d\n", fd);
	} else if (!strcmp(path, "/dev/kgsl-2d0")) {
		file_table[fd].is_2d = 1;
		printf("found kgsl_2d0: %d\n", fd);
	} else if (!strcmp(path, "/dev/kgsl-2d1")) {
		file_table[fd].is_2d = 1;
		printf("found kgsl_2d1: %d\n", fd);
	} else if (strstr(path, "/dev/")) {
		printf("#### missing device, path: %s: %d\n", path, fd);
	}
}

int open(const char* path, int flags, ...)
{
	mode_t mode = 0;
	int ret;
	PROLOG(open);

	if (flags & (O_CREAT | O_TMPFILE)) {
		va_list args;

		va_start(args, flags);
		mode = (mode_t) va_arg(args, int);
		va_end(args);

		ret = orig_open(path, flags, mode);
	} else {
#ifdef FAKE
		const char *actual_path = path;
		if (access(path, F_OK) && (path == strstr(path, "/dev/"))) {
			/* fake non-existant device files: */
			printf("emulating: %s\n", path);
			actual_path = "/dev/null";
		}
		ret = orig_open(actual_path, flags);
		if ((ret != -1) && (path != actual_path)) {
			assert(ret < ARRAY_SIZE(file_table));
			file_table[ret].is_emulated = 1;
		}
#else
		ret = orig_open(path, flags);
#endif
	}

	LOCK();

	if (ret != -1) {
		install_fd(path, ret);
	}

	UNLOCK();

	return ret;
}

int openat(int dirfd, const char *path, int flags, ...)
{
	mode_t mode = 0;
	int ret;
	PROLOG(openat);

	if (flags & (O_CREAT | O_TMPFILE)) {
		va_list args;

		va_start(args, flags);
		mode = (mode_t) va_arg(args, int);
		va_end(args);

		ret = orig_openat(dirfd, path, flags, mode);
	} else {
#ifdef FAKE
		const char *actual_path = path;
		if (access(path, F_OK) && (path == strstr(path, "/dev/"))) {
			/* fake non-existant device files: */
			printf("emulating: %s\n", path);
			actual_path = "/dev/null";
		}
		ret = orig_openat(dirfd, actual_path, flags);
		if ((ret != -1) && (path != actual_path)) {
			assert(ret < ARRAY_SIZE(file_table));
			file_table[ret].is_emulated = 1;
		}
#else
		ret = orig_openat(dirfd, path, flags);
#endif
	}

	LOCK();

	if (ret != -1) {
		install_fd(path, ret);
	}

	UNLOCK();

	return ret;
}

int __openat(int dirfd, const char *path, int flags, int mode)
{
	int ret;
	PROLOG(__openat);

	printf("openat: path: %s\n", path);

	if (flags & (O_CREAT | O_TMPFILE)) {
		ret = orig___openat(dirfd, path, flags, mode);
	} else {
#ifdef FAKE
		const char *actual_path = path;
		if (access(path, F_OK) && (path == strstr(path, "/dev/"))) {
			/* fake non-existant device files: */
			printf("emulating: %s\n", path);
			actual_path = "/dev/null";
		}
		ret = orig___openat(dirfd, actual_path, flags, mode);
		if ((ret != -1) && (path != actual_path)) {
			assert(ret < ARRAY_SIZE(file_table));
			file_table[ret].is_emulated = 1;
		}
#else
		ret = orig___openat(dirfd, path, flags, mode);
#endif
	}

	LOCK();

	if (ret != -1) {
		install_fd(path, ret);
	}

	UNLOCK();

	return ret;
}

int close(int fd)
{
	PROLOG(close);

	LOCK();

	if ((fd >= 0) && (fd < ARRAY_SIZE(file_table))) {
		if (file_table[fd].is_3d) {
			// XXX unregister buffers
			printf("closing 3d\n");
		}
		file_table[fd].is_3d = 0;
		file_table[fd].is_2d = 0;
#ifdef FAKE
		file_table[fd].is_emulated = 0;
#endif
	}

	UNLOCK();

	return orig_close(fd);
}

static void log_gpuaddr(uint64_t gpuaddr, uint32_t len)
{
	uint32_t sect[3] = {
			/* upper 32b of gpuaddr added after len for backwards compat */
			gpuaddr, len, gpuaddr >> 32,
	};
	rd_write_section(RD_GPUADDR, sect, sizeof(sect));
}

static void log_cmdaddr(uint64_t gpuaddr, uint32_t sizedwords)
{
	uint32_t sect[3] = {
			/* upper 32b of gpuaddr added after len for backwards compat */
			gpuaddr, sizedwords, gpuaddr >> 32,
	};
	rd_write_section(RD_CMDSTREAM_ADDR, sect, sizeof(sect));
}

static void dump_ib_prep(void)
{
	struct buffer *other_buf;

	list_for_each_entry(other_buf, &buffers_of_interest, node) {
		other_buf->dumped = 0;
	}
}

static void dump_ib(struct kgsl_ibdesc *ibdesc)
{
	struct buffer *buf = find_buffer(NULL, ibdesc->gpuaddr, 0, 0, 0);
	if (buf && buf->hostptr) {
		struct buffer *other_buf;
		uint32_t off = ibdesc->gpuaddr - buf->gpuaddr;
		uint32_t *ptr = buf->hostptr + off;

		printf("\t\tcmd: (%u dwords)\n", (uint32_t)ibdesc->sizedwords);

		hexdump_dwords(ptr, ibdesc->sizedwords);

		list_for_each_entry(other_buf, &buffers_of_interest, node) {
			if (other_buf && other_buf->hostptr && !other_buf->dumped) {
				uint32_t len = other_buf->len;
				log_gpuaddr(other_buf->gpuaddr, len);
				rd_write_section(RD_BUFFER_CONTENTS,
						other_buf->hostptr, len);
				other_buf->dumped = 1;
			}
		}

		/* we already dump all the buffer contents, so just need
		 * to dump the address/size of the cmdstream:
		 */
		log_cmdaddr(ibdesc->gpuaddr, ibdesc->sizedwords);
	}
}

static void dump_cmd(struct kgsl_command_object *cmd)
{
	/* note: kgsl seems to ignore cmd->offset.. which may be a bug.. */
	struct buffer *buf = find_buffer(NULL, cmd->gpuaddr, 0, 0, 0);
	if (buf && buf->hostptr) {
		struct buffer *other_buf;
		uint32_t sizedwords = cmd->size / 4;
		uint32_t off = cmd->gpuaddr - buf->gpuaddr;
		uint32_t *ptr = buf->hostptr + off;

		printf("\t\tcmd: (%u dwords)\n", sizedwords);

		hexdump_dwords(ptr, sizedwords);

		list_for_each_entry(other_buf, &buffers_of_interest, node) {
			if (other_buf && other_buf->hostptr && !other_buf->dumped) {
				uint32_t len = other_buf->len;
				log_gpuaddr(other_buf->gpuaddr, len);
				rd_write_section(RD_BUFFER_CONTENTS,
						other_buf->hostptr, len);
				other_buf->dumped = 1;
			}
		}

		/* we already dump all the buffer contents, so just need
		 * to dump the address/size of the cmdstream:
		 */
		log_cmdaddr(cmd->gpuaddr, sizedwords);
	}
}

static void kgsl_ioctl_ringbuffer_issueibcmds_pre(int fd,
		struct kgsl_ringbuffer_issueibcmds *param)
{
	int is2d = get_kgsl_info(fd) == &kgsl_2d_info;
	int i;
	struct kgsl_ibdesc *ibdesc;
	dump_ib_prep();
	printf("\t\tdrawctxt_id:\t%08x\n", param->drawctxt_id);
	/*
For z180_cmdstream_issueibcmds():

#define KGSL_CONTEXT_SAVE_GMEM	1
#define KGSL_CONTEXT_NO_GMEM_ALLOC	2
#define KGSL_CONTEXT_SUBMIT_IB_LIST	4
#define KGSL_CONTEXT_CTX_SWITCH	8
#define KGSL_CONTEXT_PREAMBLE	16

#define Z180_STREAM_PACKET_CALL 0x7C000275   <-- seems to be always first 4 bytes..

if there isn't a context switch, skip the first PACKETSIZE_STATESTREAM words:

PACKETSIZE_STATE:
	#define NUMTEXUNITS             4
	#define TEXUNITREGCOUNT         25
	#define VG_REGCOUNT             0x39

	#define PACKETSIZE_BEGIN        3
	#define PACKETSIZE_G2DCOLOR     2
	#define PACKETSIZE_TEXUNIT      (TEXUNITREGCOUNT * 2)
	#define PACKETSIZE_REG          (VG_REGCOUNT * 2)
	#define PACKETSIZE_STATE        (PACKETSIZE_TEXUNIT * NUMTEXUNITS + \
					 PACKETSIZE_REG + PACKETSIZE_BEGIN + \
					 PACKETSIZE_G2DCOLOR)

		((25 * 2) * 4 + (0x39 * 2) + 3 + 2) =>
		((25 * 2) * 4 + (57 * 2) + 3 + 2) =>
		319

PACKETSIZE_STATESTREAM:
	#define x  (ALIGN((PACKETSIZE_STATE * \
					 sizeof(unsigned int)), 32) / \
					 sizeof(unsigned int))

	ALIGN((PACKETSIZE_STATE * sizeof(unsigned int)), 32) / sizeof(unsigned int) =>
	1280 / 4 =>
	320 => 0x140

so the context, restored on context switch, is the first: 320 (0x140) words
	*/
	printf("\t\tflags:\t\t%08x\n", param->flags);
	printf("\t\tnumibs:\t\t%08x\n", param->numibs);
	printf("\t\tibdesc_addr:\t%08x\n", param->ibdesc_addr);
	ibdesc = (struct kgsl_ibdesc *)param->ibdesc_addr;
	for (i = 0; i < param->numibs; i++) {
		// z180_cmdstream_issueibcmds or adreno_ringbuffer_issueibcmds
		printf("\t\tibdesc[%d].ctrl:\t\t%08x\n", i, ibdesc[i].ctrl);
		printf("\t\tibdesc[%d].sizedwords:\t%08x\n", i, (uint32_t)ibdesc[i].sizedwords);
		printf("\t\tibdesc[%d].gpuaddr:\t%08x\n", i, ibdesc[i].gpuaddr);
		printf("\t\tibdesc[%d].hostptr:\t%p\n", i, ibdesc[i].hostptr);
		if (is2d) {
			if (ibdesc[i].sizedwords > PACKETSIZE_STATESTREAM) {
				unsigned int len, *ptr;
				/* note: kernel side seems to expect param->timestamp to
				 * contain same thing as ibdesc[0].hostptr ... this seems to
				 * be what actually gets read from on kernel side.  Maybe a
				 * legacy thing??
				 * Update: this seems to be needed so z180_cmdstream_issueibcmds()
				 * can patch up the cmdstream to jump back to the next ringbuffer
				 * entry.
				 */
				printf("\t\tcontext:\n");
				hexdump_dwords(ibdesc[i].hostptr, PACKETSIZE_STATESTREAM);
				rd_write_section(RD_CONTEXT, ibdesc[i].hostptr,
						PACKETSIZE_STATESTREAM * sizeof(unsigned int));

				printf("\t\tcmd:\n");
				ptr = (unsigned int *)(ibdesc[i].hostptr +
						PACKETSIZE_STATESTREAM * sizeof(unsigned int));
				len = ptr[2] & 0xfff;
				/* 5 is length of first packet, 2 for the two 7f000000's */
				hexdump_dwords(ptr, len + 5 + 2);
				rd_write_section(RD_CMDSTREAM, ptr,
						(len + 5 + 2) * sizeof(unsigned int));
				/* dump out full buffer in case I need to go back and check
				 * if I missed something..
				 */
				dump_buffer(ibdesc[i].gpuaddr);
			} else {
				printf("\t\tWARNING: INVALID CONTEXT!\n");
				hexdump_dwords(ibdesc[i].hostptr, ibdesc[i].sizedwords);
			}
		} else {
			dump_ib(&ibdesc[i]);
		}
	}
}

static void kgsl_ioctl_ringbuffer_issueibcmds_post(int fd,
		struct kgsl_ringbuffer_issueibcmds *param)
{
	printf("\t\ttimestamp:\t%08x\n", param->timestamp);
}

static void kgsl_ioctl_submit_commands_pre(int fd,
		struct kgsl_submit_commands *param)
{
	int i;
	struct kgsl_ibdesc *ibdesc;

	dump_ib_prep();

	ibdesc = (struct kgsl_ibdesc *)param->cmdlist;

	printf("\t\tdrawctxt_id:\t%08x\n", param->context_id);
	printf("\t\tflags:\t\t%08x\n", param->flags);
	printf("\t\tnumibs:\t\t%08x\n", param->numcmds);
	for (i = 0; i < param->numcmds; i++) {
		printf("\t\tibdesc[%d].ctrl:\t\t%08x\n", i, ibdesc[i].ctrl);
		printf("\t\tibdesc[%d].sizedwords:\t%08x\n", i, (uint32_t)ibdesc[i].sizedwords);
		printf("\t\tibdesc[%d].gpuaddr:\t%08x\n", i, ibdesc[i].gpuaddr);
		printf("\t\tibdesc[%d].hostptr:\t%p\n", i, ibdesc[i].hostptr);
		dump_ib(&ibdesc[i]);
	}
}

static void kgsl_ioctl_submit_commands_post(int fd,
		struct kgsl_submit_commands *param)
{
	printf("\t\ttimestamp:\t%08x\n", param->timestamp);
}

static void kgsl_ioctl_drawctxt_create_pre(int fd,
		struct kgsl_drawctxt_create *param)
{
	printf("\t\tflags:\t\t%08x\n", param->flags);
}

static void kgsl_ioctl_drawctxt_create_post(int fd,
		struct kgsl_drawctxt_create *param)
{
#ifdef FAKE
	static unsigned ctxid = 0;
	param->drawctxt_id = ++ctxid;
#endif
	printf("\t\tdrawctxt_id:\t%08x\n", param->drawctxt_id);
}

#define PROP_INFO(n) [n] = #n
static const char *propnames[] = {
		PROP_INFO(KGSL_PROP_DEVICE_INFO),
		PROP_INFO(KGSL_PROP_DEVICE_SHADOW),
		PROP_INFO(KGSL_PROP_DEVICE_POWER),
		PROP_INFO(KGSL_PROP_SHMEM),
		PROP_INFO(KGSL_PROP_SHMEM_APERTURES),
		PROP_INFO(KGSL_PROP_MMU_ENABLE),
		PROP_INFO(KGSL_PROP_INTERRUPT_WAITS),
		PROP_INFO(KGSL_PROP_VERSION),
		PROP_INFO(KGSL_PROP_GPU_RESET_STAT),
		PROP_INFO(KGSL_PROP_PWRCTRL),
		PROP_INFO(KGSL_PROP_PWR_CONSTRAINT),
		PROP_INFO(KGSL_PROP_UCHE_GMEM_VADDR),
		PROP_INFO(KGSL_PROP_SP_GENERIC_MEM),
		PROP_INFO(KGSL_PROP_UCODE_VERSION),
		PROP_INFO(KGSL_PROP_GPMU_VERSION),
		PROP_INFO(KGSL_PROP_HIGHEST_BANK_BIT),
		PROP_INFO(KGSL_PROP_DEVICE_BITNESS),
		PROP_INFO(KGSL_PROP_DEVICE_QDSS_STM),
		PROP_INFO(KGSL_PROP_MIN_ACCESS_LENGTH),
		PROP_INFO(KGSL_PROP_UBWC_MODE),
		PROP_INFO(KGSL_PROP_DEVICE_QTIMER),
};

static void kgsl_ioctl_device_getproperty_post(int fd,
		struct kgsl_device_getproperty *param)
{
	const char *typename =
		(param->type < ARRAY_SIZE(propnames)) ? propnames[param->type] : NULL;
	printf("\t\ttype:\t\t%08x (%s)\n", param->type,
			typename ? typename : "unknown");
	if (param->type == KGSL_PROP_DEVICE_INFO) {
		struct kgsl_devinfo *devinfo = param->value;
		uint32_t gpu_id;
		if (wrap_gpu_id()) {
			uint32_t gpu_id = wrap_gpu_id();
			/* convert gpu-id into chip-id, and add optional patch level: */
			unsigned core  = gpu_id / 100;
			unsigned major = (gpu_id % 100) / 10;
			unsigned minor = gpu_id % 10;
			int patch = wrap_gpu_id_patchid();
			devinfo->gpu_id = gpu_id;
			devinfo->chip_id = (patch & 0xff) |
					((minor & 0xff) << 8) |
					((major & 0xff) << 16) |
					((core & 0xff) << 24);
#ifdef FAKE
			devinfo->device_id = 1;
			devinfo->mmu_enabled = 1;
			devinfo->gmem_gpubaseaddr = 0x10000;
#endif
			printf("\t\tEMULATING gpu_id: %d (%08x)!!!\n",
					devinfo->gpu_id, devinfo->chip_id);
		}
		if (wrap_gmem_size()) {
			devinfo->gmem_sizebytes = wrap_gmem_size();
			printf("\t\tEMULATING gmem_sizebytes: %u !!!\n", (uint32_t)devinfo->gmem_sizebytes);
		}
		gpu_id = devinfo->gpu_id;
		if (!gpu_id) {
			gpu_id = ((devinfo->chip_id >> 24) & 0xff) * 100 +
				((devinfo->chip_id >> 16) & 0xff) * 10 +
				((devinfo->chip_id >> 8) & 0xff) * 1;
		}
		rd_write_section(RD_GPU_ID, &gpu_id, sizeof(gpu_id));
		printf("\t\tgpu_id: %d\n", gpu_id);
		printf("\t\tgmem_sizebytes: 0x%x\n", (uint32_t)devinfo->gmem_sizebytes);
#ifdef FAKE
	} else if (param->type == KGSL_PROP_DEVICE_SHADOW) {
		struct kgsl_shadowprop *shadow = param->value;
		shadow->gpuaddr = 0xc0009000;
		shadow->size = 0x2000;
		shadow->flags = 0x00000204;
	} else if (param->type == KGSL_PROP_UCHE_GMEM_VADDR) {
		uint64_t *value = param->value;
		*value = 0x10000;
		// TODO probably should return an error for a4xx and prior..
	} else if (param->type == KGSL_PROP_HIGHEST_BANK_BIT) {
		uint32_t *value = param->value;
		*value = 15;    // qcom,highest-bank-bit
	} else if (param->type == KGSL_PROP_DEVICE_BITNESS) {
		uint32_t *value = param->value;
		if (is64b) {
			*value = 48;
		} else {
			*value = 32;
		}
	} else if (param->type == KGSL_PROP_UCODE_VERSION) {
		struct kgsl_ucode_version *ucode = param->value;
		unsigned gpu_id = wrap_gpu_id();
		if (gpu_id >= 600) {
			ucode->pfp = 0x016ee166;  /* SQE version goes in pfp slot */
			ucode->pm4 = 0;
		} else {
			ucode->pfp = 0x005ff110;
			ucode->pm4 = 0x005ff066;
		}
//	} else if (param->type == KGSL_PROP_DEVICE_QDSS_STM) {
//	} else if (param->type == KGSL_PROP_DEVICE_QTIMER) {
	} else if (param->type == KGSL_PROP_MIN_ACCESS_LENGTH) {
		uint32_t *value = param->value;
		*value = 32;    // qcom,min-access-length
	} else if (param->type == KGSL_PROP_UBWC_MODE) {
		uint32_t *value = param->value;
		*value = 2;     // qcom,ubwc-mode
#endif
	}
	hexdump(param->value, param->sizebytes);
}

static int len_from_vma(unsigned int hostptr)
{
	long long addr, endaddr, offset, inode;
	FILE *f;
	int ret;

	// TODO: only for debug..
	if (0)
		dumpfile("/proc/self/maps");

	f = fopen("/proc/self/maps", "r");

	do {
		char c;
		ret = fscanf(f, "%llx-%llx", &addr, &endaddr);
		if (addr == hostptr)
			return endaddr - addr;
		/* find end of line.. we could do this more cleverly w/ glibc.. :-( */
		while (((ret = fscanf(f, "%c", &c)) > 0) && (c != '\n'));
	} while (ret > 0);
	return -1;
}

static void kgsl_ioctl_sharedmem_from_vmalloc_pre(int fd,
		struct kgsl_sharedmem_from_vmalloc *param)
{
	int len;

	/* just make gpuaddr == hostptr.. should make it easy to track */
	printf("\t\tflags:\t\t%08x\n", param->flags);
	printf("\t\thostptr:\t%08x\n", param->hostptr);
	if (param->gpuaddr) {
		len = param->gpuaddr;
	} else {
		/* note: if gpuaddr not specified, need to figure out length from
		 * vma.. that is nasty!
		 */
		len = len_from_vma(param->hostptr);

		/* for 2d/z180, all of the 0x5000 length buffers seem to be what
		 * will be passed back in IOCTL_KGSL_RINGBUFFER_ISSUEIBCMDS cmds
		 *
		 * Additional buffers that seem to be something other than RGB
		 * surfaces with lengths:
		 *
		 *   0x00001000
		 *   0x00009000
		 *   0x00081000
		 *   0x00001000
		 *   0x00009000
		 *   0x00081000
		 *
		 * These are created before the sequence of 6 0x5000 len bufs
		 * on the first c2dCreateSurface() call (which seems to be
		 * triggering some initialization).
		 *
		 * Possibly sets of two due to having both kgsl-2d0 and kgsl-2d1?
		 * But the 0x5000 buffers passed in to ISSUEIBCMDS are round-
		 * robin'd even though all ISSUEIBCMDS go to kgsl-2d1..
		 *
		 * gpu addresses seem to be consistent from run to run.. (at least
		 * as long as sequence of c2d calls doesn't change)  Currently:
		 *
		 *   00001000 - 6601a000-6601b000
		 *   00009000 - 6601c000-66025000
		 *   00081000 - 66026000-660a7000
		 *
		 *   00001000 - 660a8000-660a9000
		 *   00009000 - 660aa000-660b3000
		 *   00081000 - 660b4000-66135000
		 *
		 *   00005000 - 66136000-6613b000
		 *   00005000 - 6613c000-66141000
		 *   00005000 - 66142000-66147000
		 *   00005000 - 66148000-6614d000
		 *   00005000 - 6614e000-66153000
		 *   00005000 - 66154000-66159000
		 *
		 * Hmm, interesting that each mapping ends up with a page between..
		 *
		 * Last buffer allocated is the surface (64x64x4bpp -> 0x4000):
		 *   00004000 - 6615a000-6615e000
		 *
		 * Then for c2dReadSurface() another surface is created (no longer
		 * with extra page in between!?):
		 *   00004000 - 6615e000-66162000
		 *
		 *     4053C000  75 02 00 7C  00 00 00 00  05 00 05 00  29 01 00 7C    |u..|........)..||
		 *     4053C010 <00 A0 01 66> 2A 01 00 7C <00 C0 01 66> 2B 01 00 7C    |...f*..|...f+..||
		 *     4053C020 <00 60 02 66> 0F 01 00 7C  0A 00 00 00  08 01 00 7C    |.`.f...|.......||
		 *     4053C030  00 F0 03 00  09 01 00 7C  00 F0 03 00  00 01 00 7C    |.......|.......||
		 *     4053C040 <00 E0 15 66> 01 01 00 7C  08 70 00 00  10 01 00 7C    |...f...|.p.....||
		 *     4053C050  FF FF FF 00  D0 01 00 7C  00 00 00 00  D4 01 00 7C    |.......|.......||
		 *     4053C060  00 00 00 00  0C 01 00 7C  00 00 00 00  0E 01 00 7C    |.......|.......||
		 *     4053C070  02 00 00 00  0D 01 00 7C  04 04 00 00  0B 01 00 7C    |.......|.......||
		 *     4053C080  00 00 00 FF  0A 01 00 7C  00 00 00 FF  11 01 00 7C    |.......|.......||
		 *     4053C090  00 00 00 00  14 01 00 7C  00 00 00 00  15 01 00 7C    |.......|.......||
		 *     4053C0A0  00 00 00 00  16 01 00 7C  00 00 00 00  17 01 00 7C    |.......|.......||
		 *     4053C0B0  00 00 00 00  18 01 00 7C  00 00 00 00  19 01 00 7C    |.......|.......||
		 *     4053C0C0  00 00 00 00  1A 01 00 7C  00 00 00 00  1B 01 00 7C    |.......|.......||
		 *     4053C0D0  00 00 00 00  1C 01 00 7C  00 00 00 00  1D 01 00 7C    |.......|.......||
		 *     4053C0E0  00 00 00 00  1E 01 00 7C  00 00 00 00  1F 01 00 7C    |.......|.......||
		 *     4053C0F0  00 00 00 00  24 01 00 7C  00 00 00 00  25 01 00 7C    |....$..|....%..||
		 *     4053C100  00 00 00 00  27 01 00 7C  00 00 00 00  28 01 00 7C    |....'..|....(..||
		 *     4053C110  00 00 00 00  5E 01 00 7B  00 00 00 00  61 01 00 7B    |....^..{....a..{|
		 *     4053C120  00 00 00 00  65 01 00 7B  00 00 00 00  66 01 00 7B    |....e..{....f..{|
		 *     4053C130  00 00 00 00  6E 01 00 7B  00 00 00 00  6F 01 00 7C    |....n..{....o..||
		 *     4053C140  00 00 00 00  65 01 00 7B  00 00 00 00  54 01 00 7B    |....e..{....T..{|
		 *     4053C150  00 00 00 00  55 01 00 7B  00 00 00 00  53 01 00 7B    |....U..{....S..{|
		 *     4053C160  00 00 00 00  68 01 00 7B  00 00 00 00  60 01 00 7B    |....h..{....`..{|
		 *     4053C170  00 00 00 00  50 01 00 7B  00 00 00 00  56 01 00 7B    |....P..{....V..{|
		 *     4053C180  00 00 00 00  57 01 00 7B  00 00 00 00  58 01 00 7B    |....W..{....X..{|
		 *     4053C190  00 00 00 00  59 01 00 7B  00 00 00 00  52 01 00 7B    |....Y..{....R..{|
		 *     4053C1A0  00 00 00 00  51 01 00 7B  00 00 00 00  56 01 00 7B    |....Q..{....V..{|
		 *     4053C1B0  00 00 00 00  7F 01 00 7C  00 00 00 00  7F 01 00 7C    |.......|.......||
		 *     4053C1C0  00 00 00 00  7F 01 00 7C  00 00 00 00  7F 01 00 7C    |.......|.......||
		 *     4053C1D0  00 00 00 00  00 00 00 7F  00 00 00 7F  29 01 00 7C    |............)..||
		 *     4053C1E0 <00 80 0A 66> 2A 01 00 7C <00 A0 0A 66> 2B 01 00 7C    |...f*..|...f+..||
		 *     4053C1F0 <00 40 0B 66> E2 01 00 7C  00 00 00 00  E3 01 00 7C    |.@.f...|.......||
		 *     4053C200  00 00 00 00  E4 01 00 7C  00 00 00 00  E5 01 00 7C    |.......|.......||
		 *     4053C210  00 00 00 00  E6 01 00 7C  00 00 00 00  E7 01 00 7C    |.......|.......||
		 *     4053C220  00 00 00 00  C0 01 00 7C  00 00 00 00  C1 01 00 7C    |.......|.......||
		 *     4053C230  00 00 00 00  C2 01 00 7C  00 00 00 00  C3 01 00 7C    |.......|.......||
		 *     4053C240  00 00 00 00  C4 01 00 7C  00 00 00 00  C5 01 00 7C    |.......|.......||
		 *
		 * Some addresses seen in the state info (first 0x140 words of
		 * ISSUEIBCMDS):
		 *     00 A0 01 66 -> 6601a000  <-- 1st 0x1000 buffer
		 *     00 C0 01 66 -> 6601c000  <-- 1st 0x9000 buffer
		 *     00 60 02 66 -> 66026000  <-- 1st 0x81000 buffer
		 *     00 E0 15 66 -> 6615e000  <-- previous read surface (but already free'd at this point!)
		 *     00 80 0A 66 -> 660a8000  <-- 2nd 0x1000 buffer
		 *     00 A0 0A 66 -> 660aa000  <-- 2nd 0x9000 buffer
		 *     00 40 0B 66 -> 660b4000  <-- 2nd 0x81000 buffer
		 *
		 * From dumps of these memories before every ISSUEIBCMDS:
		 *     6601a000: - same contents across all dumps (all 00's and aa's)
		 *     6601c000: - same contents across all dumps (all 00's and aa's)
		 *     66026000: - same contents across all dumps (all 00's and aa's)
		 *     660a8000: - same contents across all dumps (all 00's and aa's)
		 *     660aa000: - same contents across all dumps (all 00's and aa's)
		 *     660b4000: - same contents across all dumps (all 00's and aa's)
		 *   (possibly these are just used for more advanced operations, or 3d only?)
		 *
		 */

		/* these buffer sizes are interesting for 2d.. not sure about 3d.. */
		switch(len) {
		case 0x5000:
//		case 0x1000:
//		case 0x9000:
//		case 0x81000:
			/* register buffer of interest */
			register_buffer((void *)param->hostptr, param->flags, len, 0);
			break;
		}
	}
#ifdef FAKE
	param->gpuaddr = alloc_gpuaddr(len);
#endif
	printf("\t\tlen:\t\t%08x\n", len);
}

static void kgsl_ioctl_sharedmem_from_vmalloc_post(int fd,
		struct kgsl_sharedmem_from_vmalloc *param)
{
	struct buffer *buf = find_buffer((void *)param->hostptr, 0, 0, 0, 0);
	log_gpuaddr(param->gpuaddr, len_from_vma(param->hostptr));
	if (buf)
		buf->gpuaddr = param->gpuaddr;
	printf("\t\tgpuaddr:\t%08x\n", param->gpuaddr);
}

static void kgsl_ioctl_sharedmem_free_pre(int fd,
		struct kgsl_sharedmem_free *param)
{
	struct buffer *buf = find_buffer((void *)-1, param->gpuaddr, 0, 0, 0);
	printf("\t\tgpuaddr:\t%08x\n", param->gpuaddr);
	unregister_buffer(buf);
}

static void kgsl_ioctl_gpumem_alloc_pre(int fd,
		struct kgsl_gpumem_alloc *param)
{
	printf("\t\tflags:\t\t%08x\n", param->flags);
	printf("\t\tsize:\t\t%08x\n", (uint32_t)param->size);
}

static void kgsl_ioctl_gpumem_alloc_post(int fd,
		struct kgsl_gpumem_alloc *param)
{
	struct buffer *buf;
	log_gpuaddr(param->gpuaddr, param->size);
	printf("\t\tgpuaddr:\t%08lx\n", param->gpuaddr);
	/* NOTE: host addr comes from mmap'ing w/ gpuaddr as offset */
	buf = register_buffer(NULL, param->flags, param->size, 0);
	buf->gpuaddr = param->gpuaddr;
	buf->offset = param->gpuaddr;
}

static void kgsl_ioctl_gpumem_alloc_id_pre(int fd,
		struct kgsl_gpumem_alloc_id *param)
{
	printf("\t\tflags:\t\t%08x\n", param->flags);
	printf("\t\tsize:\t\t%08x\n", (uint32_t)param->size);
	/* easier to force it not to USE_CPU_MAP than dealing with
	 * the mmap dance:
	 */
	param->flags &= ~KGSL_MEMFLAGS_USE_CPU_MAP;
}

static void kgsl_ioctl_gpumem_alloc_id_post(int fd,
		struct kgsl_gpumem_alloc_id *param)
{
	struct buffer *buf;
#ifdef FAKE
	static int id = 0;

	param->id = ++id;
	param->mmapsize = ALIGN(param->size, 0x1000);
	param->gpuaddr = alloc_gpuaddr(param->mmapsize);
#endif

	log_gpuaddr(param->gpuaddr, param->size);
	printf("\t\tid:\t%u\n", param->id);
	printf("\t\tgpuaddr:\t%08lx\n", param->gpuaddr);
	/* NOTE: host addr comes from mmap'ing w/ gpuaddr as offset */
	buf = register_buffer(NULL, param->flags, param->size, 0);
	buf->id = param->id;
	buf->gpuaddr = param->gpuaddr;
	buf->offset = param->gpuaddr;
}

static void kgsl_ioctl_gpumem_free_id_pre(int fd,
		struct kgsl_gpumem_free_id *param)
{
	printf("\t\tid:\t%u\n", param->id);
}

static void kgsl_ioctl_gpumem_free_id_post(int fd,
		struct kgsl_gpumem_free_id *param)
{
	struct buffer *buf = find_buffer((void *)-1, 0, 0, 0, param->id);
	unregister_buffer(buf);
}

static void kgls_ioctl_perfcounter_get_post(int fd,
		struct kgsl_perfcounter_get *param)
{
	char buf[128];

	printf("\t\tgroupid:\t%u\n", param->groupid);
	printf("\t\tcountable:\t%u\n", param->countable);
#ifdef FAKE
	int g = param->groupid % 128;
	int c = param->countable % 128;
	static struct {
		uint32_t hi, lo;
	} cache[128][128];
	if (cache[g][c].lo == 0) {
		static int off;

		if (!off) {
			if (wrap_gpu_id() >= 500) {
				off = 0x03a0;  // REG_A5XX_RBBM_PERFCTR_CP_0_LO
			} else {
				off = 0x9c;    // REG_A4XX_RBBM_PERFCTR_CP_0_LO
			}
		}

		cache[g][c].lo = off;
		cache[g][c].hi = off + 1;
	}
	param->offset = cache[g][c].lo;
	param->offset_hi = cache[g][c].hi;
#endif
	printf("\t\toffset_lo:\t0x%x\n", param->offset);
	printf("\t\toffset_hi:\t0x%x\n", param->offset_hi);

	rd_write_section(RD_CMD, buf, snprintf(buf, sizeof(buf),
			"perfcounter_get: groupid=%u, countable=%u, off_lo=0x%x, off_hi=0x%x",
			param->groupid, param->countable, param->offset, param->offset_hi));
}

static void kgls_ioctl_perfcounter_put_pre(int fd,
		struct kgsl_perfcounter_put *param)
{
	char buf[128];

	printf("\t\tgroupid:\t%u\n", param->groupid);
	printf("\t\tcountable:\t%u\n", param->countable);

	rd_write_section(RD_CMD, buf, snprintf(buf, sizeof(buf),
			"perfcounter_put: groupid=%u, countable=%u",
			param->groupid, param->countable));
}

static void kgls_ioctl_gpuobj_alloc_pre(int fd,
		struct kgsl_gpuobj_alloc *param)
{
	printf("\t\tflags:\t\t%08x %08x\n", (uint32_t)(param->flags >> 32), (uint32_t)param->flags);
	printf("\t\tsize:\t\t%08x\n", (uint32_t)param->size);
	/* easier to force it not to USE_CPU_MAP than dealing with
	 * the mmap dance:
	 */
	param->flags &= ~KGSL_MEMFLAGS_USE_CPU_MAP;
}

static void kgls_ioctl_gpuobj_alloc_post(int fd,
		struct kgsl_gpuobj_alloc *param)
{
	struct buffer *buf;
#ifdef FAKE
	static int id = 0;

	param->id = ++id;
	param->mmapsize = ALIGN(param->size, 0x1000);
#endif
	printf("\t\tid:\t%u\n", param->id);
	/* NOTE: host addr comes from mmap'ing w/ gpuaddr as offset */
	buf = register_buffer(NULL, param->flags, param->size, 0);
	buf->id = param->id;
}

static void kgls_ioctl_gpuobj_free_pre(int fd,
		struct kgsl_gpuobj_free *param)
{
	printf("\t\tid:\t%u\n", param->id);
}

static void kgls_ioctl_gpuobj_free_post(int fd,
		struct kgsl_gpuobj_free *param)
{
	struct buffer *buf = find_buffer((void *)-1, 0, 0, 0, param->id);
	unregister_buffer(buf);
}

static void kgsl_ioclt_gpuobj_info_pre(int fd,
		struct kgsl_gpuobj_info *param)
{
	printf("\t\tid:\t%u\n", param->id);
}

static void kgsl_ioclt_gpuobj_info_post(int fd,
		struct kgsl_gpuobj_info *param)
{
	struct buffer *buf = find_buffer((void *)-1, 0, 0, 0, param->id);
#ifdef FAKE
	param->size = buf->len;
	param->gpuaddr = alloc_gpuaddr(ALIGN(buf->len, 0x1000));
	param->va_addr = param->gpuaddr;
	param->va_len = buf->len;
#endif

	log_gpuaddr(param->gpuaddr, param->size);
	printf("\t\tid:\t%u\n", param->id);
	printf("\t\tgpuaddr:\t%08lx\n", param->gpuaddr);
	buf->gpuaddr = param->gpuaddr;
	buf->offset = param->gpuaddr;
}

static void kgls_ioctl_gpuobj_gpu_command_pre(int fd,
		struct kgsl_gpu_command *param)
{
	int i;
	struct kgsl_command_object *cmdobj;

	dump_ib_prep();

	cmdobj = (struct kgsl_command_object *)param->cmdlist;

	printf("\t\tdrawctxt_id:\t%08x\n", param->context_id);
	printf("\t\tflags:\t\t%08x %08x\n", (uint32_t)(param->flags >> 32), (uint32_t)param->flags);
	printf("\t\tnumcmds:\t\t%08x\n", param->numcmds);

	for (i = 0; i < param->numcmds; i++) {
		printf("\t\tcmd[%d].flags:\t\t%08x\n", i, cmdobj[i].flags);
		printf("\t\tcmd[%d].sizedwords:\t%08x\n", i, (uint32_t)cmdobj[i].size / 4);
		printf("\t\tcmd[%d].gpuaddr:\t%08x\n", i, cmdobj[i].gpuaddr);
		dump_cmd(&cmdobj[i]);
	}
}

static void kgls_ioctl_gpuobj_gpu_command_post(int fd,
		struct kgsl_gpu_command *param)
{
	printf("\t\ttimestamp:\t%08x\n", param->timestamp);
}

static void kgsl_ioctl_pre(int fd, unsigned long int request, void *ptr)
{
	dump_ioctl(get_kgsl_info(fd), _IOC_WRITE, fd, request, ptr, 0);
	switch(_IOC_NR(request)) {
	case _IOC_NR(IOCTL_KGSL_RINGBUFFER_ISSUEIBCMDS):
		kgsl_ioctl_ringbuffer_issueibcmds_pre(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_SUBMIT_COMMANDS):
		kgsl_ioctl_submit_commands_pre(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_DRAWCTXT_CREATE):
		kgsl_ioctl_drawctxt_create_pre(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_SHAREDMEM_FROM_VMALLOC):
		kgsl_ioctl_sharedmem_from_vmalloc_pre(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_SHAREDMEM_FREE):
		kgsl_ioctl_sharedmem_free_pre(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_GPUMEM_ALLOC):
		kgsl_ioctl_gpumem_alloc_pre(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_GPUMEM_ALLOC_ID):
		kgsl_ioctl_gpumem_alloc_id_pre(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_GPUMEM_FREE_ID):
		kgsl_ioctl_gpumem_free_id_pre(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_PERFCOUNTER_PUT):
		kgls_ioctl_perfcounter_put_pre(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_GPUOBJ_ALLOC):
		kgls_ioctl_gpuobj_alloc_pre(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_GPUOBJ_FREE):
		kgls_ioctl_gpuobj_free_pre(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_GPUOBJ_INFO):
		kgsl_ioclt_gpuobj_info_pre(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_GPU_COMMAND):
		kgls_ioctl_gpuobj_gpu_command_pre(fd, ptr);
		break;
	}
}

static void kgsl_ioctl_post(int fd, unsigned long int request, void *ptr, int ret)
{
	dump_ioctl(get_kgsl_info(fd), _IOC_READ, fd, request, ptr, ret);
	switch(_IOC_NR(request)) {
	case _IOC_NR(IOCTL_KGSL_RINGBUFFER_ISSUEIBCMDS):
		kgsl_ioctl_ringbuffer_issueibcmds_post(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_SUBMIT_COMMANDS):
		kgsl_ioctl_submit_commands_post(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_DRAWCTXT_CREATE):
		kgsl_ioctl_drawctxt_create_post(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_DEVICE_GETPROPERTY):
		kgsl_ioctl_device_getproperty_post(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_SHAREDMEM_FROM_VMALLOC):
		kgsl_ioctl_sharedmem_from_vmalloc_post(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_GPUMEM_ALLOC):
		kgsl_ioctl_gpumem_alloc_post(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_GPUMEM_ALLOC_ID):
		kgsl_ioctl_gpumem_alloc_id_post(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_GPUMEM_FREE_ID):
		kgsl_ioctl_gpumem_free_id_post(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_PERFCOUNTER_GET):
		kgls_ioctl_perfcounter_get_post(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_GPUOBJ_ALLOC):
		kgls_ioctl_gpuobj_alloc_post(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_GPUOBJ_FREE):
		kgls_ioctl_gpuobj_free_post(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_GPUOBJ_INFO):
		kgsl_ioclt_gpuobj_info_post(fd, ptr);
		break;
	case _IOC_NR(IOCTL_KGSL_GPU_COMMAND):
		kgls_ioctl_gpuobj_gpu_command_post(fd, ptr);
		break;
	}
}

// XXX android/bionic has messed up ioctl signature:
int ioctl(int fd, int request, ...)
{
	int ioc_size = _IOC_SIZE(request);
	int ret;
	PROLOG(ioctl);
	void *ptr;

	// XXX fbdev doesn't appear to play by the rules:
	ioc_size = 1;

	if (ioc_size) {
		va_list args;

		va_start(args, request);
		ptr = va_arg(args, void *);
		va_end(args);
	} else {
		ptr = NULL;
	}

	LOCK();

	if (!get_kgsl_info(fd)) {
		char path[64];
		char buf[256];
		int ret;

		sprintf(path, "/proc/self/fd/%d", fd);

		ret = readlink(path, buf, sizeof(buf));
		if (ret > 0) {
			buf[ret] = '\0';
			install_fd(buf, fd);
		}
	}

	if (get_kgsl_info(fd))
		kgsl_ioctl_pre(fd, request, ptr);
	else
		printf("> [%4d]         : <unknown> (%08lx)\n", fd, (long)request);

	if ((_IOC_NR(request) == _IOC_NR(IOCTL_KGSL_RINGBUFFER_ISSUEIBCMDS)) &&
			get_kgsl_info(fd) && wrap_safe()) {
		sync();
		sleep(1);
	}

	UNLOCK();

#ifdef FAKE
	if (file_table[fd].is_emulated) {
#else
	if (((_IOC_NR(request) == _IOC_NR(IOCTL_KGSL_RINGBUFFER_ISSUEIBCMDS)) ||
			(_IOC_NR(request) == _IOC_NR(IOCTL_KGSL_SUBMIT_COMMANDS)) ||
			(_IOC_NR(request) == _IOC_NR(IOCTL_KGSL_GPU_COMMAND)) ||
			(_IOC_NR(request) == _IOC_NR(IOCTL_KGSL_DEVICE_WAITTIMESTAMP)) ||
			(_IOC_NR(request) == _IOC_NR(IOCTL_KGSL_DEVICE_WAITTIMESTAMP_CTXTID))) &&
			get_kgsl_info(fd) && (wrap_gpu_id() || wrap_gmem_size())) {
#endif
		/* don't actually submit cmds to hw.. because we are pretending to
		 * be something different from the actual hw
		 */
		ret = 0;
	} else {
		ret = orig_ioctl(fd, request, ptr);
	}

	LOCK();

	if (get_kgsl_info(fd))
		kgsl_ioctl_post(fd, request, ptr, ret);
	else
		printf("< [%4d]         : <unknown> (%08lx) (%d)\n", fd, (long)request, ret);

	UNLOCK();

	if ((_IOC_NR(request) == _IOC_NR(IOCTL_KGSL_RINGBUFFER_ISSUEIBCMDS)) &&
			get_kgsl_info(fd) && wrap_safe()) {
		sync();
		sleep(1);
	}

	return ret;
}

void * mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	void *ret = NULL;
	PROLOG(mmap);

	LOCK();

	if ((fd >= 0) && get_kgsl_info(fd)) {
		//struct buffer *buf = find_buffer(NULL, 0, offset, 0, 0);
		struct buffer *buf = find_buffer(NULL, 0, 0, 0, offset >> 12); // XXX only id's are used now

		printf("< [%4d]         : mmap: addr=%p, length=%u, prot=%x, flags=%x, offset=%08lx\n",
				fd, addr, (uint32_t)length, prot, flags, offset);

		if (buf && buf->hostptr) {
			buf->munmap = 0;
			ret = buf->hostptr;
		}
	}

	if (!ret) {
#ifdef FAKE
		if ((fd >= 0) && file_table[fd].is_emulated) {
			ret = malloc(length);
		} else {
			ret = orig_mmap(addr, length, prot, flags, fd, offset);
		}
#else
		ret = orig_mmap(addr, length, prot, flags, fd, offset);
#endif
	}

	if ((fd >= 0) && get_kgsl_info(fd)) {
		//struct buffer *buf = find_buffer(NULL, 0, offset, 0, 0);
		struct buffer *buf = find_buffer(NULL, 0, 0, 0, offset >> 12); // XXX only id's are used now
		if (buf)
			buf->hostptr = ret;
		else {
			/*
			 * when a buffer is allocated using IOCTL_KGSL_GPUMEM_ALLOC_ID
			 * it's mmapped by id, not by gpuaddr, so try to find that
			 * buffer via id now.
			 */
			buf = find_buffer(NULL, 0, 0, 0, offset >> 12);
			if (buf)
				buf->hostptr = ret;
		}
		printf("< [%4d]         : mmap: -> (%p)\n", fd, ret);
	}

	UNLOCK();

	return ret;
}

void *mmap64(void *addr, size_t length, int prot, int flags, int fd, int64_t offset)
{
	void *ret = NULL;
	PROLOG(mmap64);

	LOCK();

	if ((fd >= 0) && get_kgsl_info(fd)) {
		//struct buffer *buf = find_buffer(NULL, 0, offset, 0, 0);
		struct buffer *buf = find_buffer(NULL, 0, 0, 0, offset >> 12); // XXX only id's are used now

		printf("< [%4d]         : mmap64: addr=%p, length=%u, prot=%x, flags=%x, offset=%08lx\n",
				fd, addr, (uint32_t)length, prot, flags, offset);

		if (buf && buf->hostptr) {
			printf("  [%4d]	    : (recycled from buf=%p)\n", fd, buf);
			buf->munmap = 0;
			ret = buf->hostptr;
		}
	}

	if (!ret) {
#ifdef FAKE
		if ((fd >= 0) && file_table[fd].is_emulated) {
			ret = calloc(1, length);
		} else {
			ret = orig_mmap64(addr, length, prot, flags, fd, offset);
		}
#else
		ret = orig_mmap64(addr, length, prot, flags, fd, offset);
#endif
	}

	if ((fd >= 0) && get_kgsl_info(fd)) {
		//struct buffer *buf = find_buffer(NULL, 0, offset, 0, 0);
		struct buffer *buf = find_buffer(NULL, 0, 0, 0, offset >> 12); // XXX only id's are used now
		if (buf)
			buf->hostptr = ret;
		else {
			/*
			 * when a buffer is allocated using IOCTL_KGSL_GPUMEM_ALLOC_ID
			 * it's mmapped by id, not by gpuaddr, so try to find that
			 * buffer via id now.
			 */
			buf = find_buffer(NULL, 0, 0, 0, offset >> 12);
			if (buf)
				buf->hostptr = ret;
		}
		printf("< [%4d]         : mmap64: -> (%p), buf=%p\n", fd, ret, buf);
	}

	UNLOCK();

	return ret;
}

int munmap(void *addr, size_t length)
{
	struct buffer *buf;
	int ret;
	PROLOG(munmap);

	LOCK();

	buf = find_buffer(addr, 0, 0, 0, 0);
	if (buf) {
		/* we need the contents at submit ioctl: */
printf("fake munmap: buf=%p\n", buf);
		buf->munmap = 1;
		ret = 0;
		goto out;
	}

	ret = orig_munmap(addr, length);
out:
	UNLOCK();
	return ret;
}
