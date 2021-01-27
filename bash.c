#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define DOUBLE_MORE 1
#define MF_SIZE 5

struct words {
	char *word;
	int spec_symb;
	struct words *next;
};

struct item {
	int val;
	struct item *next;
};

struct pids {
	int pid;
	struct pids *next;
};

enum FLAGS {
	WORD,
	QUOTES,
	SPEC_SYMB,
	PREV_SYMB,
	SYMB,
};

enum errors {
	no_error,
	err_ampersand,
	err_quotes,
	err_less,
	err_more,
	err_d_more,
	err_delim,
	err_consecutive_spec_symb,
	err_more_and_d_more,
};


void output_pids(struct pids *pid_list)
{
	while(pid_list!=NULL) {
		printf("pid=%d\n",pid_list->pid);
		pid_list=pid_list->next;
	}
}

void sigchld_handler(int s)
{
	while(wait4(-1,NULL,WNOHANG,NULL)>0) {
	}
}

void error_messages(int error)
{
	switch(error) {
		case err_ampersand : {
			fprintf(stderr,"ICHBash: Syntax error with «&»\n");
			break;
		}
		case err_more : {
			fprintf(stderr,"ICHBash: Syntax error with «>»\n");
			break;
		}
		case err_less : {
			fprintf(stderr,"ICHBash: Syntax error with «<»\n");
			break;
		}
		case err_d_more : {
			fprintf(stderr,"ICHBash: Syntax error with «>>»\n");
			break;
		}
		case err_quotes : {
			fprintf(stderr,
				"ICHBash : Syntax error with the quotes\n");
			break;
		}
		case err_delim : {
			fprintf(stderr,"ICHBash: Syntax error with «|»\n");
			break;
		}
		case err_consecutive_spec_symb : {
			fprintf(stderr,"ICHBash: Syntax error" 
				" with several consecutive special symbols\n");
			break;
		}
		case err_more_and_d_more : {
			fprintf(stderr,"ICHBash: Syntax error" 
				" with using «>>» and «>» together\n");
			break;
		}
	}
}

char *get_dir()
{
	int i=1;
	char *dir;
	dir=malloc(i);
	while(getcwd(dir,i)==NULL) {
		free(dir);
		i=i*2;
		dir=malloc(i);
	}
	return dir;
}

void promt_to_enter()
{
	char *dir;
	printf("ivan@ICHISTYAKOV:%s$ ",dir=get_dir());
	free(dir);
}

void massif_flags_cleaning(int *massif_flags)
{
	int i;
	for(i=0;i<MF_SIZE;i++) {
		massif_flags[i]=0;
	}
}
	

void free_item(struct item **word_list)
{		
	struct item *word_tmp;
	while(*word_list!=NULL) {
		word_tmp=*word_list;
		*word_list=(*word_list)->next;
		free(word_tmp);
	}
}

void free_pids(struct pids **pid_list)
{		
	struct pids *pid_tmp;
	while(*pid_list!=NULL) {
		pid_tmp=*pid_list;
		*pid_list=(*pid_list)->next;
		free(pid_tmp);
	}
}

void free_words(struct words **list)
{
	struct words *tmp;
	while(*list!=NULL) {
		tmp=*list;
		*list=(*list)->next;
		free(tmp->word);
		free(tmp);
	}
}

void set_flags(int *massif_flags)
{
	int symb=massif_flags[SYMB];
	if(symb=='"') {
		if(massif_flags[QUOTES]==1)
			massif_flags[QUOTES]=0;
		else
			massif_flags[QUOTES]=1;
	}
	if(massif_flags[QUOTES]==0) {
		switch(symb) {
			case '&': {
				massif_flags[SPEC_SYMB]='&';
				break;
			}
			case '|': {
				massif_flags[SPEC_SYMB]='|';
				break;
			}				
			case '<': {
				massif_flags[SPEC_SYMB]='<';
				break;
			}
			case '>': {
				massif_flags[SPEC_SYMB]='>';
				break;
			}
  			default:
				massif_flags[SPEC_SYMB]=0;
		}				
	}
}

int check_end_of_word(int *massif_flags)
{
	int flag=-1, symb=massif_flags[SYMB];
	if(symb==' '||symb=='\t'||symb=='\n') {
		if(massif_flags[WORD]==1&&massif_flags[QUOTES]==0) {
			flag=0;
		}
	}
	return flag;
}

int check_end_of_input(int *massif_flags)
{
	int flag;
	if(massif_flags[QUOTES]==0) {
		flag=no_error;
	} else {
		flag=err_quotes;
	}
	return flag;
}

int check_create_list(int *massif_flags)
{
	int flag=-1, symb=massif_flags[SYMB];
	if(symb==' '||symb=='\t'||symb=='\n'||massif_flags[SPEC_SYMB]>0) {
		if((massif_flags[WORD]==1||massif_flags[SPEC_SYMB]>0)
		&&massif_flags[QUOTES]==0) {
			flag=0;
		}
	}
	return flag;
}

int check_word(int *massif_flags)
{
	int i=-1, symb=massif_flags[SYMB];
	if(massif_flags[QUOTES]==0&&massif_flags[SPEC_SYMB]==0) {
		if(symb!=' '&&symb!='\n'&&symb!='\t'&&symb!='"') 
			i=0;	
	}
	if(symb!='"'&&massif_flags[QUOTES]==1) {
		i=0;
	}
	return i;
}

void read_word(struct item **word_list,int *massif_flags)
{
	static struct item *tmp;
	int symb=massif_flags[SYMB];
	if(*word_list==NULL) {
		*word_list=malloc(sizeof(struct item));
		tmp=*word_list;
	} else {
		tmp->next=malloc(sizeof(struct item));
		tmp=tmp->next;
	}
	tmp->val=symb;
	tmp->next=NULL;
	massif_flags[WORD]=1;
}

int list_length(struct words *list)
{
	int i=0;
	while(list!=NULL) {
		i++;
		list=list->next;
	}
	return i;
}

int list_length_only_words(struct words *list) 
{
	int i=0, word_after_ss=0;
	while(list!=NULL) {
		if(list->spec_symb=='|') {
			break;
		}
		if(list->spec_symb==0) {
			if(word_after_ss==0) {
				i++;
			} else {
				word_after_ss=0;
			}
		} else {
			if(word_after_ss==0)
				word_after_ss=1;
		}
		list=list->next;
	}
	return i;
}

int word_length(struct item *word_list)
{
	int i=0;
	while(word_list!=NULL) {
		i++;
		word_list=word_list->next;
	}
	return i;
}

char *fill_word(struct item *word_list)
{
	int k=0;
	char *word;
	word=malloc(word_length(word_list)+1);
	while(word_list!=NULL) {
		word[k]=word_list->val;
		word_list=word_list->next;
		k++;
	}
	word[k]='\0';
	return word;
}

void several_consecutive_spec_symb(struct words **list,int *massif_flags)
{
	struct words *tmp=*list;
	if(massif_flags[SYMB]=='>'&&tmp->spec_symb=='>') {
		if(massif_flags[PREV_SYMB]=='>') {
			tmp->spec_symb=DOUBLE_MORE;
		} else {
			tmp->spec_symb=-1;
		}
	} else {
		tmp->spec_symb=-1;
	}
	*list=tmp;
}


void fill_list(struct words **list,struct item *word_list,int *massif_flags)
{
	static struct words *tmp;
	if(*list==NULL) {
		*list=malloc(sizeof(struct words));
		tmp=*list;
		tmp->spec_symb=massif_flags[SPEC_SYMB];
	} else {
		if(massif_flags[SPEC_SYMB]!=0&&tmp->spec_symb!=0) {
			several_consecutive_spec_symb(&tmp,massif_flags);
		} else {
			tmp->next=malloc(sizeof(struct words));
			tmp=tmp->next;
			tmp->spec_symb=massif_flags[SPEC_SYMB];
		}
	}
	tmp->word=fill_word(word_list);
	tmp->next=NULL;
}

void create_list(struct words **list,struct item **word_list,int *massif_flags)
{
	struct words *tmp=*list;
	struct item *word_tmp=*word_list;
	int spec_symb=massif_flags[SPEC_SYMB];
	if(massif_flags[SPEC_SYMB]==0) {	
		fill_list(&tmp,word_tmp,massif_flags);
	} else {
		if(word_tmp!=NULL) {
			massif_flags[SPEC_SYMB]=0;
			fill_list(&tmp,word_tmp,massif_flags);
			word_tmp=NULL;
			massif_flags[SPEC_SYMB]=spec_symb;
			fill_list(&tmp,word_tmp,massif_flags);
		} else {
			fill_list(&tmp,word_tmp,massif_flags);	
		}
	}
	massif_flags[WORD]=0;
	massif_flags[SPEC_SYMB]=0;
	*list=tmp;
}

int read_string(struct words **main_list,int *massif_flags)
{
	struct item *word_list=NULL;
	struct words *list=NULL;
	while((massif_flags[SYMB]=getchar())!=EOF) {
		set_flags(massif_flags);
		if(check_word(massif_flags)==0) {
			read_word(&word_list,massif_flags);
		}
		if(check_create_list(massif_flags)==0) {
			create_list(&list,&word_list,massif_flags);
			free_item(&word_list);
		}
		if(massif_flags[SYMB]=='\n') {
			*main_list=list;
			free_item(&word_list);
			return 0;
		}
		massif_flags[PREV_SYMB]=massif_flags[SYMB];
	}
	return -1;
}

void output_list(struct words *list)
{
	while(list!=NULL) {
		printf("%s\n",list->word);
		printf("%d\n",list->spec_symb);
		list=list->next;
	}
}	

char **command_line(struct words *list)
{
	char **cmd;
	int i=0, word_after_ss=0;
	cmd=malloc((list_length_only_words(list)+1)*sizeof(char*));
	while(list!=NULL) {
		if(list->spec_symb=='|') {
			break;
		}
		if(list->spec_symb==0) { 
			if(word_after_ss==0) {
				cmd[i]=list->word;
				i++;
			} else {
				word_after_ss=0;
			}
		} else {
			if(word_after_ss==0) {
				word_after_ss=1;
			}
		}
		list=list->next;
	}
	cmd[i]=NULL;
	return cmd;
}

void cd_process(char **cmd)
{
	if(cmd[1]==NULL) {
		fprintf(stderr,"ICHBash: cd: Too few arguments\n");
	} else {
		if(cmd[2]==NULL) {
			if(chdir(cmd[1])!=0)
				perror(cmd[1]);
		} else {
			fprintf(stderr,"ICHBash: cd: Too many arguments\n");
		}	
	}
}

int check_ampersand(struct words *list)
{
	int ampersand=0;
	while(list!=NULL) {
		if(list->spec_symb=='&')
			ampersand=1;
		list=list->next;
	}
	return ampersand;
}

int check_sign(struct words *list,int c)
{
	int flag=0;
	while(list!=NULL) {
		if(list->spec_symb==c) {
			flag=1;
			break;
		}
		list=list->next;
	}
	return flag;
}

char *return_filename(struct words *list,int c)
{
	char *filename;
	while(list!=NULL) {
		if(list->spec_symb==c) {
			list=list->next;
			filename=list->word;
		}
		list=list->next;
	}
	return filename;
}

void redirecting_input(struct words *list,int c)
{
	int fd;
	char *filename;
	filename=return_filename(list,c);
	if(c=='>') {
		fd=open(filename,O_CREAT|O_WRONLY|O_TRUNC,0666);
	} else {
		fd=open(filename,O_CREAT|O_APPEND|O_WRONLY,0666);
	}
	if(fd==-1) {
		perror(filename);
		exit(1);
	}
	dup2(fd,1);
	close(fd);
}
	
void redirecting_output(struct words *list,int c)
{
	int fd;
	char *filename;
	filename=return_filename(list,c);
	fd=open(filename,O_RDONLY,0666);
	if(fd==-1) {
		perror(filename);
		exit(1);
	}
	dup2(fd,0);
	close(fd);
}

void child_process(char **cmd,struct words *list,int i,int quant_delim)
{
	if(check_sign(list,'<')==1&&i==0) {
		redirecting_output(list,'<');
	}
	if(check_sign(list,'>')==1&&i==quant_delim) {
		redirecting_input(list,'>');
	}
	if(check_sign(list,DOUBLE_MORE)==1&&i==quant_delim) {
		redirecting_input(list,DOUBLE_MORE);
	}
	execvp(cmd[0],cmd);
	perror(cmd[0]);
	fflush(stderr);
	_exit(1);
}

void error_process()
{
	fprintf(stderr,"ICHBash: Error with the fork\n");
	perror("fork");
	exit(1);
}

struct words *correct_list(struct words *list)
{
	while(list!=NULL) {
		if(list->spec_symb=='|') {
			list=list->next;
			break;
		}
		list=list->next;
	}
	return list;
}

void create_pid_list(struct pids **pid_list,int pid)
{
	static struct pids *pid_tmp;
	if(*pid_list==NULL) {
		*pid_list=malloc(sizeof(struct pids));
		pid_tmp=*pid_list;
	} else {
		pid_tmp->next=malloc(sizeof(struct pids));
		pid_tmp=pid_tmp->next;
	}
	pid_tmp->pid=pid;
	pid_tmp->next=NULL;
}

struct pids *delete_zombie_pid(struct pids *pid_list,int zombie)
{
	struct pids **tmp=&pid_list;
	while(*tmp!=NULL) {
		if((*tmp)->pid==zombie) {
			*tmp=(*tmp)->next;
		} else {
			tmp=&(*tmp)->next;
		}
	}
	return pid_list;
}

void parent_process(struct words *list,struct pids *pid_list)
{
	int zombie;
	if(check_ampersand(list)==0) {
		signal(SIGCHLD,SIG_DFL);
		while(pid_list!=NULL) {
			zombie=wait(NULL);
			pid_list=delete_zombie_pid(pid_list,zombie);	
		}
		signal(SIGCHLD,sigchld_handler);
	}
}

void handling_fd_in_child(int *fd,int *fd_in,int i,int quant_delim)
{
	if(i==0) {
		dup2(fd[1],1);
	}
	if(i>0&&i<quant_delim) {
		dup2(*fd_in,0);
		close(*fd_in);
		dup2(fd[1],1);
	}
	if(i==quant_delim) {
		dup2(*fd_in,0);
		close(*fd_in);
	}
	close(fd[0]);
	close(fd[1]);
}

void handling_fd_in_parent(int *fd,int *fd_in)
{
	if(*fd_in!=-1) {
		close(*fd_in);
	}
	close(fd[1]);
	*fd_in=dup(fd[0]);
	close(fd[0]);
}

void new_process(struct words *list,int quant_delim)
{
	int fd[2];
	int i, pid, fd_in=-1;
	struct pids *pid_list=NULL;
	struct words *tmp=list;
	for(i=0;i<=quant_delim;i++) {
		char **cmd=command_line(tmp);
		if(quant_delim!=0) 
			pipe(fd);
		pid=fork();
		if(pid==0) {
			if(quant_delim!=0) 
				handling_fd_in_child(fd,&fd_in,i,quant_delim);
			child_process(cmd,list,i,quant_delim);
		}
		if(pid>0) {
			if(quant_delim!=0)
				handling_fd_in_parent(fd,&fd_in);
			create_pid_list(&pid_list,pid);
			if(i==quant_delim) {
				if(quant_delim!=0) {
					close(fd_in);
				}
				parent_process(list,pid_list);
			}	
		}
		if(pid==-1) {
			error_process();
		}
		tmp=correct_list(tmp);
		free(cmd);
	}
	free_pids(&pid_list);
}

int quantity_delimiters(struct words *list)
{
	int quant=0;
	while(list!=NULL) {
		if(list->spec_symb=='|') {
			quant++;
		}
		list=list->next;
	}
	return quant;
}

int check_chdir(char **cmd)
{
	if(cmd[0]!=NULL) {
		if (cmd[0][0]=='c'&&cmd[0][1]=='d'&&cmd[0][2]=='\0') {
			return 0;
		}
	}
	return 1;
}

void create_new_processes(struct words *list)
{
	char **cmd=command_line(list);
	int quant_delim=quantity_delimiters(list);
	cmd=command_line(list);
	if(check_chdir(cmd)==0) {
		cd_process(cmd);
	} else {
		new_process(list,quant_delim);
	}
	free(cmd);
}

int analyse_ampersand(struct words *list)
{
	int i=0, flag=no_error;
	struct words *tmp=list;
	while(tmp!=NULL) {
		i++;
		if(tmp->spec_symb=='&') 
			if(list_length(list)!=i||i==1) {
				flag=err_ampersand;
				break;
			}
		tmp=tmp->next;
	}
	return flag;
}

int set_flag(int c)
{
	int flag;
	if(c=='>')
		flag=err_more;
	if(c=='<')
		flag=err_less;
	if(c==DOUBLE_MORE)
		flag=err_d_more;
	if(c=='|')
		flag=err_delim;
	return flag;
}

int analyse_signs(struct words *list,int c)
{
	int i=0, flag=no_error, quant_of_signs=0;
	struct words *tmp=list;
	while(tmp!=NULL) {
		i++;
		if(tmp->spec_symb==c) {
			quant_of_signs++;
			if(c=='|'&&i==1) {
				flag=set_flag(c);
			}
			if(list_length(list)==i) {
				flag=set_flag(c);
				return flag;
			}
		}
		tmp=tmp->next;
	}
	if(c!='|'&&quant_of_signs>1) {
		flag=set_flag(c);
	}
	return flag;	
}

int analyse_consecutive_spec_symb(struct words *list)
{
	int flag=no_error;
	while(list!=NULL) {
		if(list->spec_symb==-1) {
			flag=err_consecutive_spec_symb;
			break;
		}
		list=list->next;
	}
	return flag;
}

int analyse_more_and_d_more_together(struct words *list)
{
	int flag=no_error, quant_of_more=0, quant_of_double_more=0;
	while(list!=NULL) {
		if(list->spec_symb==DOUBLE_MORE)
			quant_of_double_more++;
		if(list->spec_symb=='>')
			quant_of_more++;
		list=list->next;
	}
	if(quant_of_more>0&&quant_of_double_more>0) 
		flag=err_more_and_d_more;
	return flag;
}

int analyse_list(struct words *list)
{	
	int analyse_var=no_error;
	if(analyse_ampersand(list)==err_ampersand) {
		analyse_var=err_ampersand;
		return analyse_var;
	}
	if(analyse_consecutive_spec_symb(list)==err_consecutive_spec_symb) {
		analyse_var=err_consecutive_spec_symb;
		return analyse_var;
	}
	if(analyse_signs(list,'<')==err_less) {
		analyse_var=err_less;
		return analyse_var;
	}
	if(analyse_signs(list,'>')==err_more) {
		analyse_var=err_more;
		return analyse_var;
	}
	if(analyse_signs(list,DOUBLE_MORE)==err_d_more) {
		analyse_var=err_d_more;
		return analyse_var;
	}	
	if(analyse_signs(list,'|')==err_delim) {
		analyse_var=err_delim;
		return analyse_var;
	}
	if(analyse_more_and_d_more_together(list)==err_more_and_d_more) {
		analyse_var=err_more_and_d_more;
		return analyse_var;
	}
	return analyse_var;
}

int main()
{
	struct words *main_list=NULL;
	int massif_flags[MF_SIZE];
	signal(SIGCHLD,sigchld_handler);
	promt_to_enter();
	massif_flags_cleaning(massif_flags);
	while(read_string(&main_list,massif_flags)!=-1) {
		if(check_end_of_input(massif_flags)==no_error) {
			if(analyse_list(main_list)==no_error) {
				create_new_processes(main_list);
			} else {
				error_messages(analyse_list(main_list));
			}
		} else {
			error_messages(check_end_of_input(massif_flags));
		}
		massif_flags_cleaning(massif_flags);
		free_words(&main_list);
		promt_to_enter();	
	}
	printf("\n");
	return 0;
}
