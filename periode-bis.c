#include <stdio.h>

int MenuPeriode();
int MenuContinuer();
void EnvoiePeriode(int periode);

int main()
{
	int choixPeriode, choixContinuer = 1;
	int fd; //Fichier

	while (choixContinuer == 1) {

		choixPeriode = MenuPeriode();
		while (choixPeriode < 1 || choixPeriode > 5) {
			printf("Mauvaise Pioche");
			choixPeriode = MenuPeriode();
		}

		EnvoiePeriode(choixPeriode);		

		choixContinuer = MenuContinuer();
		while (choixContinuer < 1 || choixContinuer > 2) {
			printf("Mauvaise Pioche");
			choixContinuer = MenuContinuer();
		}
	}

	return 0;
}


int MenuPeriode() {
	int result;
	printf("\nChoisissez la periode du signal :\n1 - 50 us\n2 - 40 us\n3 - 30 us\n4 - 20 us\n5 - 10 us\n");
	scanf("%d", &result);
	return result;

}

int MenuContinuer() {
	int result;
	printf("\nVoulez-vous encore changer la periode du signal :\n1 - Oui\n2 - Non\n");
	scanf("%d", &result);
	return result;
}

void EnvoiePeriode(int periode) {
	FILE* file;

	printf("Envoie de la periode ...");
	file = fopen("/dev/rtp0", "w+");

	if (file == NULL) {
		printf("\nErreur ouverture fichier");
		return;
	}

	fputc(periode, file);
	fclose(file);

	printf("\nEnvoi de %d a /dev/rtp0 effectue", periode);
}