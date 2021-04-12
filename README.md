Nume: Filimon Adrian
Grupă: 334CC

# Tema 2 - Stdio library

Organizare
-

- Pentru stocarea datelor despre fisier am folosit structura SO_FILE cu urmatoarele campuri:

```
typedef struct _so_file {
	char buffer[4096];
	size_t buflen;
	int fd;
	int mode;
	int update;
	int nth_ch;
	int last_op;
	int is_EOF;
	int err;
	long file_offset;
	long read_offset;
} SO_FILE;
```

- abordarea temei este una destu de simpla:
    - toate operatiile sunt facute cu ajutorul apelurilor de sistem
    - pentru inchiderea fisierelor este utilizat si `flag-ul last_op`. Daca ultima operatie a fost una de scriere, trebuie sa ne asiguram ca toata informatia este scrisa in fisier si nu ramane nimic in buffer
    -  pentru scrierea si citirea datelor de la o zona de memorie am scris functiile de la 0(nu am folosit functiile fgetc si fputc tocmai pentru a avea un control mai bun asupra datelor).
        - ambele functii verifica pentru aceleasi cazuri:
            - daca buffer-ul e plin/am citit deja toate datele din buffer - se face un apel de sistem(citire/scriere)
            - daca este suficient spatiu in buffer(sau suficiente date), este utilizat doar bufferul si nu se face niciun syscall
            - daca nu este suficient spatiu in buffer, se citesc/scriu datele pana cand se ajunge la dimensiunea primita ca parametru

        - Desi sunt mai multe linii de cod, aceasta abordare mi s-a parut mai cuprinzatoare si usor de inteles

- pentru implementarea fseek, retin intr-un camp din structura offset-ul returnat de apelul de sistem lseek

### Eficienta

- functiile fread si fwrite contin cod destul de asemanator si puteau fi implementate mai elegant, dar s-ar fi pierdut din claritatea codului
- per total, tema este implementata eficient, nu folosesc heap decat pentru structura SO_FILE, atunci cand se deschide un fisierul

### Utilitate

- Tema este utila pentru intelegerea conceptelor de baza despre apelurile de sistem
- Cred ca putina creativitate nu ar fi stricat, nu este o tema neparat interesanta, dar cu siguranta este utila

Implementare
-

- Implementarea este facuta doar pentru Linux
- nu am implementat deloc partea cu procese
- am incercat sa fac popen, dar imi tot ramane hanging in testul 40

Cum se compilează și cum se rulează?
-

- fiind o biblioteca dinamica, aceasta compilata cu ajutorul Makefile-ului pus la dispozitie
- apoi sa face linkarea la codul in care se doreste utilizarea bibliotecii

Bibliografie
-

- [Laboratorul 2](https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-02)
