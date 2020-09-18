#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Attempt at making a primitive "database", practicing data structures and pointer use
struct animal
{
	int age;
	char name[25];
	char family[25];
	char breed[25];
	int code;
	
	struct animal *next;
}animal, *head, *tail, *p;

void add_pet(struct animal *animal_p);
void free_mem();
void show_pets();
void remove_pet(int code);
void save();
main()
{
	int sel;
	head = NULL;
	
	while(1)
	{
	printf("\nMenu selections\n");
	printf("------------------\n");
	printf("1. Add animal\n");
	printf("2. Show animals\n");
	printf("3. Release animal\n");
	printf("4. Save archive\n");
	printf("5. Exit\n");
	printf("Select from the menu: ");
	scanf("%d", &sel);
	
	switch(sel)
	{
		case 1:
			printf("Input the information of the animal you want to add.\n");
			getchar();
			
			printf("Animal: ");
			gets(animal.family);
			
			printf("Breed: ");
			gets(animal.breed);
			
			printf("Name: ");
			gets(animal.name);
			
			printf("Age: ");
			scanf("%d", &animal.age);
			
			printf("I.D.: ");
			scanf("%d", &animal.code);
			getchar();
			
			add_pet(&animal);
		break;
		
		case 2:
			if(head != NULL)
				show_pets();
			else
				printf("\nThere are no animals!\n");
		break;
		
		case 3:
			printf("Input the code of the released animal: ");
			scanf("%d", &animal.code);
			remove_pet(animal.code);
		break;
		
		case 4:
			if(head != NULL)
				save();
			else
				printf("\nThere are no animals!\n");
		break;
		
		case 5:
			printf("\nExiting program!!\n");
			if(head != NULL)
				free_mem();
			exit(1);
		break;
		default:
			printf("\nWrong choice!!\n");
		break;
	}
	}
}
	
void add_pet(struct animal *animal_p)
{
	struct animal *new_animal;
	
	new_animal = (struct animal *)malloc(sizeof(struct animal));
	
	if(new_animal == NULL)
	{
		printf("Error: no available memory!");
		exit(1);
	}
	*new_animal = *animal_p;
	new_animal->next = head;
	head = new_animal;
}
		
void free_mem()
{
	struct animal *new_animal;
	p = head;
	
	do
	{
		new_animal = p->next;
		free(p);
		p = new_animal;
	}while(p != NULL);
}

void show_pets()
{	
	p = head;
	
	printf("\n****Animals in list****\n");
	do
	{
		printf("I.D.: %d\n", p->code);
		printf("Name: %s\n", p->name);
		printf("Animal: %s\n", p->family);
		printf("Breed: %s\n", p->breed);
		printf("Age: %d\n", p->age);
		p = p->next;
		printf("\n\n");
	}while(p != NULL);
}

void remove_pet(int code)
{
	struct animal *prev;
	
	p = prev = head;
	int n;
	n = 0;
	
	if(head == NULL)
	{
		printf("\nThere are no animals to release!\n");
		return;
	}
	
	do
	{
		if(p->code == code)
		{
			n = 1;
			if(p == head)
			{
				head = p->next;
			}
			else
			{
				prev->next = p->next;
				if(p == tail)
					tail = prev;
			}
			free(p);
			printf("\nThe animal with code %d, was released!\n", code);
			return;
		}
		prev = p;
		p = p->next;
		
	}while(p != NULL);
	
	if(n != 1)
			printf("\nThe animal with code %d, was not found in archive!\n", code);
}

void save()
{
	FILE *fp;
	char savefile[100];
	p = head;
	
	printf("\nWhere would you like to save it?\n");
	scanf("%s", savefile);
	fp = fopen(savefile, "w+");
	
	fprintf(fp,"\n****Animals in list****\n");
	do
	{
		fprintf(fp, "I.D.: %d\n", p->code);
		fprintf(fp, "Name: %s\n", p->name);
		fprintf(fp, "Animal: %s\n", p->family);
		fprintf(fp, "Breed: %s\n", p->breed);
		fprintf(fp, "Age: %d\n", p->age);
		p = p->next;
		fprintf(fp, "\n\n");
	}while(p != NULL);
	
	fclose(fp);
}

