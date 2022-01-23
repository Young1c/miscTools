/**
 * func_offset - Get the symbol offset within the
 *               binary/library
 *
 * Compile:
 *     gcc -lelf func_offset.c -o func_offset
 */
#include <limits.h>
#include <fcntl.h>
#include <gelf.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char *func = NULL;
static char *file = NULL;

static void usage()
{
	printf(
		"func_offset -f <func name> -p <binary path>\n"
	);
	exit(-1);
}

/*
 * Leveraged from uprobe_helpers.c of iovisor/bcc project.
 */
loff_t symbol_offset()
{
	size_t shstrndx, nhdrs;
	GElf_Shdr shdr[1];
	GElf_Sym sym[1];
	Elf_Data *data;
	GElf_Ehdr ehdr;
	GElf_Phdr phdr;
	off_t ret = -1;
	int i, fd = -1;
	Elf_Scn *scn;
	char *n;
	Elf *e;

	if (elf_version(EV_CURRENT) == EV_NONE) {
		fprintf(stderr, "elf init failed\n");
		return -1;
	}

	fd = open(file, O_RDONLY);
	if (fd < 0)
		return -1;

	e = elf_begin(fd, ELF_C_READ, NULL);
	if (!e)
		goto close_fd;

	if (!gelf_getehdr(e, &ehdr))
		goto out;

	if (elf_getshdrstrndx(e, &shstrndx) != 0)
		goto out;

	scn = NULL;
	while ((scn = elf_nextscn(e, scn))) {
		if (!gelf_getshdr(scn, shdr))
			continue;
		if (!(shdr->sh_type == SHT_SYMTAB || shdr->sh_type == SHT_DYNSYM))
			continue;
		data = NULL;
		while ((data = elf_getdata(scn, data))) {
			for (i = 0; gelf_getsym(data, i, sym); i++) {
				n = elf_strptr(e, shdr->sh_link, sym->st_name);
				if (!n)
					continue;
				if (GELF_ST_TYPE(sym->st_info) != STT_FUNC)
					continue;
				if (!strcmp(n, func)) {
					ret = sym->st_value;
					goto check;
				}
			}
		}
	}

check:
	if (ehdr.e_type == ET_EXEC || ehdr.e_type == ET_DYN) {
		if (elf_getphdrnum(e, &nhdrs) != 0) {
			ret = -1;
			goto out;
		}
		for (i = 0; i < (int)nhdrs; i++) {
			if (!gelf_getphdr(e, i, &phdr))
				continue;
			if (phdr.p_type != PT_LOAD || !(phdr.p_flags & PF_X))
				continue;
			if (phdr.p_vaddr <= ret && ret < (phdr.p_vaddr + phdr.p_memsz)) {
				ret = ret - phdr.p_vaddr + phdr.p_offset;
				goto out;
			}
		}
		ret = -1;
	}
out:
	elf_end(e);
close_fd:
	close(fd);
	return ret;
}

int main(int argc, char *argv[])
{
	loff_t offset;
	int c;

	if (argc != 5)
		usage();

	while ((c = getopt(argc, argv, "f:p:")) != EOF) {
		if (c == 'f')
			func = strdup(optarg);
		else if (c == 'p')
			file = realpath(optarg, NULL);
		else
			usage();
	}

	if (!file) {
		fprintf(stderr, "Invalid binary file path\n");
		exit(-1);
	}

	offset = symbol_offset();
	if (offset <= 0)
		fprintf(stderr, "Cannot get offset of %s\n", func);
	else
		fprintf(stdout, "The func offset of %s is %#x\n", func, offset);

	if (func)
		free(func);
	if (file)
		free(file);
	return 0;
}