#include<limits.h>
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>
#include<errno.h>

#define handle_error_en(en, msg)				\
  do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg)				\
  do { perror(msg); exit(EXIT_FAILURE); } while (0)

/* variaveis agregadas globais */
long  sum = 0;
long  odd = 0;
long  min = INT_MAX;
long  max = INT_MIN;
bool volatile done = false;


/* Inicializando mutex e variavel de condicao */

pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  mutex_list = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t   cond_list = PTHREAD_COND_INITIALIZER;

/* Estrutura da lista ligada */

struct Node {
	int value;
	struct Node* next;
};

typedef struct Node List;

List *volatile first = NULL, *volatile last = NULL, *volatile aux = NULL;

/* Prototipos de função */

void help();
void add_to_list(int val);
int remove_from_list();
void* worker_function();
void* update(long number);


/* Menu de ajuda */

void help(){
    fprintf(stdout, "\nMenu Ajuda:\n"
                    "	-f: Nome do arquivo (sem extensao).\n"
                    "	-t: Numero de threads.\n"
					"	-h: Mostra Menu Ajuda.\n"
					"\nRequisitos:\n"
                    "	1. -f e -t sao argumentos obrigatorios.\n");
}

/* Adiciona a lista */

void add_to_list(int val){

	aux = (struct Node*)malloc(sizeof(struct Node));
	aux -> value = val;
	aux -> next = NULL;
	
	if(first == NULL){
		first = aux;
		last = aux;
	} else {
		last->next = aux;
		last = aux;
	}
}


/* Remove da lista */

int remove_from_list(){
	aux = first;
	first = first->next;

	if(first == NULL){
		last = NULL;
	}

	int val = aux->value;
	return val;
}

/* Atualiza as variaveis globais agregadas, conforme o numero */

void *update(long number){

	/* Simula a computacao */
	sleep(number);

	/* Atualiza as variaveis globais protegendo-as com mutex */

	pthread_mutex_lock(&mutex);
	sum += number;
	pthread_mutex_unlock(&mutex);

	/* impares */
	if (number % 2 == 1){  
		pthread_mutex_lock(&mutex);
		odd++;
		pthread_mutex_unlock(&mutex);
	}
	/* Minimo */
	if (number < min){
		pthread_mutex_lock(&mutex);
		min = number;
		pthread_mutex_unlock(&mutex);
	}
	/* Maximo */
	if (number > max){
		pthread_mutex_lock(&mutex);
		max = number;
		pthread_mutex_unlock(&mutex);
	}

	return NULL;
}

/* Funcao executada pelas threads trabalhadoras*/

void* worker_function(){
	while(true) {
		pthread_mutex_lock(&mutex_list);
		long num;
		/* enquanto lista vazia */
		while(last == NULL && first == NULL){
			if(done){
				pthread_mutex_unlock(&mutex_list);
				return NULL;
			}
			pthread_cond_wait(&cond_list, &mutex_list);
		}
		/* remove o valor da lista a ser processado */
		if (first == NULL){
			num = 0;
		}
		else{
			num = remove_from_list();
		}
		pthread_mutex_unlock(&mutex_list);
		
		/* processa o valor */
		if(num != 0){
			update(num);
		}
	}
	pthread_exit(0);
}


int main(int argc, char **argv){
	int num_of_threads = - 1;
	int opt;
	char *fn, action;
	long num;
	
	/* Menu de opcoes */
	
	while ((opt = getopt(argc, argv, "f:t:h")) != -1){
		switch (opt){
        case 'f':
			fn = optarg;
            break;
		case 't':
            num_of_threads = strtoul(optarg, NULL, 0);
            break;
        case 'h':
            help();
            break;
		default:
            printf("\nUsage: %s [-f <Nome_do_Arquivo>] [-t <Numero_threads>]\n\n", argv[0]);
			exit(EXIT_FAILURE);
        }
    }
	
	/* Erros de argumentos */

	if(argc < 2 || num_of_threads <= 0 || fn == NULL) {
		printf("\nUsage: %s [-f <Nome_do_Arquivo>] [-t <Numero_de_threads>]\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	
	/* Iniciando criacao das threads */
	pthread_t tid[num_of_threads];
	pthread_attr_t attr;
	int s = pthread_attr_init(&attr);
	
	if (s != 0){
		handle_error_en(s, "pthread_attr_init");
	}
	
	for(int i=0; i<num_of_threads; i++){
		int s = pthread_create(&tid[i], &attr, worker_function, NULL);
		
		if (s != 0){
			handle_error_en(s, "pthread_create");
		}
	}

	/* Lendo Arquivo */
	
	FILE* fin = fopen(fn, "r");
	if( fin == NULL){
		printf("\nERRO: Arquivo não encontrado ou vazio.\nUtilize menu ajuda (-h) para mais informacoes.\n\n");
		exit(EXIT_FAILURE);
	}
	/* Carrega os numeros e adiciona-os a lista */
	
	while(fscanf(fin, "%c %ld\n", &action, &num) ==2){
		if(action == 'p'){
			pthread_mutex_lock(&mutex_list);
			add_to_list(num);
			pthread_cond_signal(&cond_list);
			pthread_mutex_unlock(&mutex_list);
		}
		else if(action == 'e'){
			sleep(num);
		}
		else{
			printf("Acao nao conhecida : '%c'\n", action);
			exit(EXIT_FAILURE);
		}
	}
	pthread_mutex_lock(&mutex_list);
	done = true;
	pthread_cond_broadcast(&cond_list);
	pthread_mutex_unlock(&mutex_list);


	/* juntando as threads */
	for(int i=0; i<num_of_threads; i++){
		int s = pthread_join(tid[i], NULL);
		if (s != 0){
			handle_error_en(s, "pthread_join");
		}
	}

	/* mostrando resultados  */
	printf("%ld %ld %ld %ld\n", sum, odd, min, max);
	
	/* limpando memoria e retornando */
	fclose(fin);
	pthread_mutex_destroy(&mutex_list);
	pthread_cond_destroy(&cond_list);
	
	return (EXIT_SUCCESS);
}