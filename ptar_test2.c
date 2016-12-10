#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <x86intrin.h>
#include <malloc.h>
//#include <windows.h>

void *aligned_alloc(size_t alignment, size_t size);

struct header_posix_ustar {
char name[100];
char mode[8];
char uid[8];
char gid[8];
char size[12];
char mtime[12];
char checksum[8];
char typeflag[1];
char linkname[100];
char magic[6];
char version[2];
char uname[32];
char gname[32];
char devmajor[8];
char devminor[8];
char prefix[155];
char pad[12];
};


int main(int argc, char* argv[]) {

	int opt;
	int flag_x, flag_l, flag_z;
	int nb_threads, flag_p;

	flag_x = 0;
	flag_l = 0;
	flag_z = 0;
	nb_threads = 0;
	flag_p = 0;

	while ((opt = getopt(argc, argv, "xlp:z")) != -1) {
		switch (opt) {

			case 'x':
				flag_x = 1;
				break;

			case 'l':
				flag_l = 1;
				break;

			case 'p':
				nb_threads = atoi(optarg);
				flag_p = 1;
				break;

			case 'z':
				flag_z = 1;
				break;

			default:

				fprintf(stderr, "Usage: %s [-x] [-l] [-p nb_threads] [-z] nom_fichier\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}


	printf("flag_x=%d; flag_l=%d; flag_p=%d; nb_threads=%d; flag_z=%d; optind=%d\n", flag_x, flag_l, flag_p, nb_threads, flag_z, optind);

	if (optind >= argc) {
		fprintf(stderr, "Expected argument after options\n");
		exit(EXIT_FAILURE);
	}

	printf("name argument = %s\n", argv[optind]);


	/*OUVERTURE DE L'ARCHIVE TAR EN LECTURE SEULE*/


	int fd;
	int nb_args;
	nb_args = flag_x + flag_l + flag_p + flag_z + nb_threads;
	fd = open(argv[nb_args+1],O_RDONLY,0);

	if (fd < 3) {
		printf("Impossible d'ouvrir le fichier\n");
		exit(1);
	} else {
		printf("le fichier %s a bien été ouvert\n", argv[1]);
		struct header_posix_ustar ma_struct;
		int nb_bits_lus=0;
	     
	 /*LECTURE DE L'ENTETE DU PREMIER FICHIER (bloc de 512 octets) */

		char* bloc_end=(char*) calloc(512,1);
		int i=0;


		while (1) {

			nb_bits_lus=pread(fd, &ma_struct, 512, i*512);

			if(!memcmp(bloc_end, &ma_struct, 512)){
				pread(fd, &ma_struct, 512, (i+1)*512);
				if(!memcmp(bloc_end, &ma_struct, 512)) {
					printf("FIN\n");
					break;
				}
			}


			printf("nom du %deme fichier : %s\n", i+1, ma_struct.name);
			printf("Nombre de bits lus : %d\n", nb_bits_lus);

			if(strcmp(ma_struct.typeflag, "0") == 0) {
				printf("Regular file\n");
			} else if(strcmp(ma_struct.typeflag, "5") == 0) {
				printf("Directory\n");
			}

			printf("mode : %s\n", ma_struct.mode);


			if (flag_x==1 && flag_l==0 && flag_p==0 && flag_z==0) {

				int mod = atoi(ma_struct.mode);
				const char * mastructname = ma_struct.name;


				if (strcmp(ma_struct.typeflag, "5") == 0) {


					mkdir(mastructname, mod);
					chmod(mastructname, mod);
					printf("nouveau mode:%d\n", mod);
					printf("extraction du document %s\n", ma_struct.name);
					//int chmod(const char * ma_struct.name, (mode_t) mod);

				} else if(strcmp(ma_struct.typeflag, "0") == 0) {


					FILE *fp;
					

					fp = fopen(ma_struct.name, "wb" );

					if(fp == NULL){
						perror("Could not open input file");
						return -1;
					} else {

					printf("derrière fwrite:%s \n", ma_struct.name);

					fchmod((intptr_t)fp, mod);
					printf("nouveau mode 1e bloc:%d\n", mod);

					//détecter si le suivant d'un fichier n'est pas un document ou un fichier
					nb_bits_lus=pread(fd, &ma_struct, 512, (i+1)*512);
					int mod = atoi(ma_struct.mode);
					fchmod((intptr_t)nb_bits_lus, mod);
					printf("nouveau mode 2e bloc:%d\n", mod);
					if (strcmp(ma_struct.typeflag, "5") != 0 && strcmp(ma_struct.typeflag, "0") != 0) {
						fprintf(fp, "%s", ma_struct.name);  //utiliser fwrite ??
						i++;
					}

					printf("size de ma_struct:%lu\n",sizeof(ma_struct));

					printf("encore ouvert");
					fclose(fp);
					printf("fermé");

					}

					/*char str[] = "This is tutorialspoint.com";
					printf("devant fwrite\n");
					fwrite(str, 1, sizeof(str), fDest);
					printf("derrière fwrite\n");
*/
					/*fclose(fDest);

					if(!reussite) {
						printf("extraction ratée du fichier %s\n", ma_struct.name);
					} else {
						printf("extraction réussie du fichier %s\n", ma_struct.name);
					}

					printf("mod:%d\n", mod);*/

				}

				printf("\n\n\n");
			}

			i++;
		}
    	}

close(fd);

/*	else if (flag_x==1 && flag_l==0 && flag_p==0 && flag_z==0) {

		int fd;
		fd = open(argv[2],O_RDONLY,0);
	 
		if (fd < 3) {
			printf("Impossible d'ouvrir le fichier\n");
			exit(1);
		} else {
			printf("le fichier %s a bien été ouvert\n", argv[2]);
			struct header_posix_ustar ma_struct;
			int nb_bits_lus=0;


			nb_bits_lus=pread(fd, &ma_struct, 512, 0);
			int mod = atoi(ma_struct.mode);
			printf("ma_struct.mode:%s\n", ma_struct.mode);
			printf("mod:%d\n", mod);
			printf("bonjour le nom du fichier est %s connard\n", ma_struct.name);

			if (strcmp(ma_struct.typeflag, "5") == 0) {

				mkdir((const char *) ma_struct.name, (mode_t) mod);

			} else {

			copier_fichier((char const * const) ma_struct.name, nb_bits_lus);

			}

		}

	close(fd);

	}
*/

return 0;
}


 

