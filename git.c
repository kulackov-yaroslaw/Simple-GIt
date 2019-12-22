#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

int size = 4100;

int GitSt() {
	int status = 0;
	struct stat st;
	if(-1 != stat("mygit", &st)) {
		status++;
	}
	if(status && S_ISDIR(st.st_mode)) {
		status++;
	}

	return status;
}

int main(int argc, char* argv[]) {
	if(!strcmp(argv[1], "init")) {
		int status_git = GitSt();
		if (status_git == 0){
			pid_t mkdir_pid = fork();
			if(mkdir_pid == 0) {
				execlp("mkdir", "mkdir", "mygit", NULL);
				exit(0);
			} else {
				int status_mkdir;
				waitpid(mkdir_pid, &status_mkdir, 0);
				pid_t make_added_pid = fork();
				if(make_added_pid == 0) {
					char buff[size];
					sprintf(buff, "mygit/%s", "added");
					execlp("touch", "touch", buff, NULL);
					exit(0);
				} else {
					int status_make_added;
					waitpid(make_added_pid, &status_make_added, 0);
					pid_t make_info_pid = fork();
					if(make_info_pid == 0) {
						char buff[size];
						sprintf(buff, "mygit/%s", "info");
						execlp("touch", "touch", buff, NULL);
						exit(0);
					}else {
						int status_make_info;
						waitpid(make_info_pid, &status_make_info, 0);
						FILE * info_file = fopen("mygit/info", "w");
						fprintf(info_file, "%d", 0);
						fclose(info_file);
					}
				}
			}
		} else {
			if(status_git == 1) {
				printf("Imossible to init, another <<mygit>> file exists\n");
			} else {
				printf("Git already initialized\n");
			}
		}
	}
	if(!strcmp(argv[1], "dell")) {
		execlp("rm", "rm", "-R", "mygit", NULL);
	}
	if(!strcmp(argv[1], "add")) {
		if(GitSt() == 2) {
			int dirfd = open("mygit", O_RDWR | O_DIRECTORY);
			FILE *added = fopen("mygit/added", "r+");
			char buff[size];
			int already_add = 0;
			while(fscanf(added, "%s", buff) > 0) {
				if(!strcmp(buff, argv[2])) {
					already_add = 1;
					break;
				}
			}
			if(!already_add) {
				fprintf(added, "%s\n", argv[2]);
			} else {
				printf("Already added\n");
			}
			fclose(added);
		} else {
			printf("Git is not initialized\n");
		}
	}
	if(!strcmp(argv[1], "commit")) {
		if(GitSt() == 2) {
			FILE* info_file = fopen("mygit/info", "r+");
			int num_commit;
			fscanf(info_file, "%d", &num_commit);
			fseek(info_file, 0, SEEK_SET);
			fprintf(info_file, "%d", num_commit + 1);
			fclose(info_file);
			pid_t mkdir_commit = fork();
			if(mkdir_commit == 0) {
				char buff[size];
				sprintf(buff, "%s/%d", "mygit", num_commit);
				execlp("mkdir", "mkdir", buff, NULL);
			} else {
				int mkdir_commit_status;
				waitpid(mkdir_commit, &mkdir_commit_status, 0);
				FILE* added_file = fopen("mygit/added", "r+");
				char file_name[size];
				while(fscanf(added_file, "%s", file_name) >  0) {
					pid_t copy_pid = fork();
					if(copy_pid == 0) {
						char new_folder[size];
						sprintf(new_folder, "%s/%d/", "mygit", num_commit);
						errno = 0;
						if(-1 == open(file_name, O_RDONLY) && errno == ENOENT) {
							exit(0);
						}
						execlp("cp", "cp", file_name, new_folder, NULL);
					} else {
						int copy_status;
						waitpid(copy_pid, &copy_status, 0);
					}
				}
				fclose(added_file);
				pid_t delete_added_pid = fork();
				if(delete_added_pid == 0) {
					execlp("rm", "rm", "mygit/added", NULL);
				} else {
					int status_delete;
					waitpid(delete_added_pid, &status_delete, 0);
					pid_t create_added = fork();
					if(create_added == 0) {
						execlp("touch", "touch", "mygit/added", NULL);
					}else {
						int status_add;
						waitpid(create_added, &status_add, 0);
						pid_t make_file_name = fork();
						char buff[size];
						sprintf(buff, "mygit/%d/commit_name", num_commit);
						if(make_file_name == 0) {
							execlp("touch", "touch", buff, NULL);
						} else {
							int create_name_st;
							waitpid(make_file_name, &create_name_st, 0);
							execlp("nano", "nano", buff, NULL);
						}
					}

				}
			}
		} else {
			printf("Git is not initialized\n");
		}
	}
	if (!strcmp(argv[1], "log")) {
		if(GitSt() == 2) {
			char name_commit[size];
			char path_name[size];
			FILE* info_file = fopen("mygit/info", "r");
			int last_idx_commit;
			fscanf(info_file, "%d", &last_idx_commit);
			last_idx_commit--;
			fclose(info_file);
			for(int idx_commit = 0; idx_commit <= last_idx_commit; idx_commit++) {
				printf("Commit ID: %d, Commit Name: ",idx_commit);
				sprintf(path_name, "mygit/%d/commit_name", idx_commit);
				FILE* file_name_com = fopen(path_name, "r");
				while(fscanf(file_name_com, "%s", name_commit) > 0) {
					printf("%s ", name_commit);
				}
				printf("\n");
				fclose(file_name_com);
			}
		} else {
			printf("Git is not initialized\n");
		}
	}
	if(!strcmp("checkout", argv[1])) {
		if(GitSt() == 2) {
			int commit_idx = atoi(argv[2]);
			FILE* info_file = fopen("mygit/info", "r");
			int last_idx_commit;
			fscanf(info_file, "%d", &last_idx_commit);
			last_idx_commit--;
			fclose(info_file);
			if(commit_idx <= last_idx_commit) {
				char dir_commit[size];
				sprintf(dir_commit, "mygit/%d/", commit_idx);
				int dirfd = open(dir_commit, O_RDONLY | O_DIRECTORY);
				DIR* dir = fdopendir(dirfd);
				struct dirent* entry;
				char file_path[size];
				while((entry = readdir(dir)) != NULL) {
					if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") || !strcmp(entry->d_name, "commit_name")) {
						continue;
					}
					sprintf(file_path, "mygit/%d/%s", commit_idx, entry->d_name);
					pid_t del_pid = fork();
					if(del_pid == 0) {
						execlp("rm", "rm", entry->d_name, NULL);
						exit(0);
					}else {
						int del_stat;
						waitpid(del_pid, &del_stat, 0);
						pid_t copy_pid = fork();
						if(copy_pid == 0) {
							execlp("cp", "cp", file_path, "./", NULL);
							exit(0);
						}else {
							int cp_status;
							waitpid(copy_pid, &cp_status, 0);
						}
					}
				}
				closedir(dir);
			} else {
				printf("Commit not found\n");
			}
		} else {
			printf("Git is not initialized\n");
		}
	}
}