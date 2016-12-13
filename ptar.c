#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <malloc.h>
#include <time.h>
#include <utime.h>
//#include <pwd.h>


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


			if (nb_args == 0) printf("nom du %deme fichier : %s\n", i+1, ma_struct.name);
			printf("Nombre de bits lus : %d\n", nb_bits_lus);


			if (flag_l==1) {


					//Affiche le type du dossier (fichier ou répertoire)
				if(strcmp(ma_struct.typeflag, "0") == 0) {
					printf("Regular file\n");
					printf("nom du fichier : %s\n", ma_struct.name);
				/*} else if(strcmp(ma_struct.typeflag, "2") == 0){
					char buf[100];
					readlink(ma_struct.name, buf, sizeof(buf));
					printf("nom du lien symbolique: %s->%s\n", ma_struct.name, buf);*/
				} else if(strcmp(ma_struct.typeflag, "5") == 0) {
					printf("Directory\n");
					printf("nom du répertoire : %s\n", ma_struct.name);
				}

					//Affiche le mode du dossier sous forme drwxr-xr-x
				char moderwx[] = "----------";

				if(strcmp(ma_struct.typeflag, "5") == 0) {
					moderwx[0]='d';
				}
				int i;
				for (i=0;i<3;i++) {
					if(ma_struct.mode[3+i]=='7'){
						moderwx[1+3*i]='r';
						moderwx[2+3*i]='w';
						moderwx[3+3*i]='x';
					} else if (ma_struct.mode[3+i]=='6') {
						moderwx[1+3*i]='r';
						moderwx[2+3*i]='w';
					} else if (ma_struct.mode[3+i]=='5') {
						moderwx[1+3*i]='r';
						moderwx[3+3*i]='x';
					} else if (ma_struct.mode[3+i]=='4') {
						moderwx[1+3*i]='r';
					} else if (ma_struct.mode[3+i]=='3') {
						moderwx[2+3*i]='w';
						moderwx[3+3*i]='x';
					} else if (ma_struct.mode[3+i]=='2') {
						moderwx[2+3*i]='w';
					} else if (ma_struct.mode[3+i]=='1') {
						moderwx[3+3*i]='x';
					}

				}

				printf("mode : %s\n", moderwx);


					//Affiche l'uid et le gid sous forme 1001/1001
				printf("uid/gid: %ld/%ld\n",strtol(ma_struct.uid, NULL, 8),strtol(ma_struct.gid, NULL, 8));

					//Affiche la taille du dossier
				int taille, j;
				int size = atoi(ma_struct.size);
				printf("size:%d\n", size);
				//sprintf(sz,"%d", size);

				int tint = sizeof(size);
				char sz[tint];
				taille=(int)sz[tint];
				printf("taille du fichier avant for:%d\n", taille);
				for (j=tint;j>=0;j--) {
					taille = taille + ((int)sz[i])*(tint-1-j)*8;
				}

/*
				printf("ma_struct.size container:%s\n", ma_struct.size);
				printf("ma_struct.name container:%s\n", ma_struct.name);
				printf("ma_struct.uid container:%s\n", ma_struct.uid);
				printf("ma_struct.gid container:%s\n", ma_struct.gid);
				printf("ma_struct.mtime container:%s\n", ma_struct.mtime);
*/



				printf("taille du fichier:%d\n", taille);
				printf("size en octal:%ld\n", strtol(ma_struct.size, NULL, 8));

				//printf("nom utilisateur (uname):%s\n", ma_struct.uname);
				//printf("nom groupe (gname):%s\n", ma_struct.gname);


					//Affiche la date de modification
				time_t time = (time_t) strtol(ma_struct.mtime, NULL, 8);
				struct tm * timeinfo;
				timeinfo=localtime(&time);
				printf("temps de modification: %d-%d-%d %d:%d:%d\n",(timeinfo->tm_year)+1900, (timeinfo->tm_mon)+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

				// CODE UTILISANT PASSWRD POUR OBTENIR NOM UTILISATEUR

/*
				struct passwd * passwd_struct;
				int test = strtol(ma_struct.uid, NULL, 8);

				if((passwd_struct = getpwuid((uid_t) test)) == NULL) {
					printf("lol NULL\n");
				} else {
				printf("nom utilisateur: %s\n", passwd_struct->pw_name);
				}

*/
			}


			if (flag_x==1 /*&& flag_l==0 && flag_p==0 && flag_z==0*/) {

				int mod = strtol(ma_struct.mode, NULL, 8);
				const char * mastructname = ma_struct.name;


				if (strcmp(ma_struct.typeflag, "5") == 0) {


					mkdir(mastructname, mod);
					chmod(mastructname, mod);
					printf("nouveau mode:%d\n", mod);
					printf("extraction du document %s\n", ma_struct.name);
					//int chmod(const char * ma_struct.name, (mode_t) mod);


					//changer la permission
					if (chmod (ma_struct.name, mod) < 0)
					    {
						fprintf(stderr, "%s: error in chmod(%s, %s) - %d (%s)\n",
							argv[0], ma_struct.name, mod, errno, strerror(errno));
						exit(1);
					    }

					printf("nouveau mode 1e bloc:%d\n", mod);


					//propriétaire+groupe
					if(chown(ma_struct.name,strtol(ma_struct.uid, NULL, 8), strtol(ma_struct.gid, NULL, 8)) == -1){
						printf("mastruct.name: %s\n", ma_struct.name);
						printf("Echec de la transmission du nom du propriétaire et du groupe:%s\n", strerror(errno));
					}

					//changer temps de modification des dossiers extraits
					struct utimbuf times;
					printf("Passé\n");
					times.modtime = (time_t) strtol(ma_struct.mtime, NULL, 8);
					printf("Passé2\n");
					if (utime(ma_struct.name, &times) == -1) {
						printf("Echec de la transmission du temps:%s\n", strerror(errno));
					}


				} else if(strcmp(ma_struct.typeflag, "0") == 0) {


					FILE *fp;
					

					fp = fopen(ma_struct.name, "wb" );

					if(fp == NULL){
						perror("Could not open input file");
						return -1;
					} else {

					printf("derrière fwrite:%s \n", ma_struct.name);

					
					//changer la permission
					if (chmod (ma_struct.name, mod) < 0)
					    {
						fprintf(stderr, "%s: error in chmod(%s, %s) - %d (%s)\n",
							argv[0], ma_struct.name, mod, errno, strerror(errno));
						exit(1);
					    }

					printf("nouveau mode 1e bloc:%d\n", mod);


					//propriétaire+groupe
					if(chown(ma_struct.name,strtol(ma_struct.uid, NULL, 8), strtol(ma_struct.gid, NULL, 8)) == -1){
						printf("mastruct.name: %s\n", ma_struct.name);
						printf("Echec de la transmission du nom du propriétaire et du groupe:%s\n", strerror(errno));
					}


					//détecter si le suivant d'un fichier n'est pas un document ou un fichier   (distinguer les fichiers vides)
					nb_bits_lus=pread(fd, &ma_struct, 512, (i+1)*512);
					int mod2 = atoi(ma_struct.mode);
					fchmod((intptr_t)nb_bits_lus, mod2);
					printf("nouveau mode 2e bloc:%d\n", mod2);

					int k;
					k=i;

					//si le bloc mémoire suivant n'est ni un fichier ni un répertoire
					while (strcmp(ma_struct.typeflag, "5") != 0 && strcmp(ma_struct.typeflag, "0") != 0) {

						//sortir du while si deux blocs vides
						if(!memcmp(bloc_end, &ma_struct, 512)){ 
							pread(fd, &ma_struct, 512, (k+1)*512);
							if(!memcmp(bloc_end, &ma_struct, 512)) {
								break;
							}
						}

						//on écrit dans le file le contenu de ma_struct.name
						fprintf(fp, "%s\n", ma_struct.name);  //utiliser fwrite ??


						k++;

						//on lit le bloc suivant
						nb_bits_lus=pread(fd, &ma_struct, 512, (k+1)*512);

					}

					nb_bits_lus=pread(fd, &ma_struct, 512, i*512);

					//changer temps de modification des dossiers extraits
					struct utimbuf times;
					printf("Passé\n");
					times.actime = (time_t) strtol(ma_struct.mtime, NULL, 8);
					times.modtime = (time_t) strtol(ma_struct.mtime, NULL, 8);
					printf("Passé2\n");
					if (utime(ma_struct.name, &times) == -1) {
						printf("Echec de la transmission du temps:%s\n", strerror(errno));
					}

					i=k;

					printf("encore ouvert");
					fclose(fp);
					printf("fermé");

					}

				}

			}
			printf("\n\n\n");
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


 

