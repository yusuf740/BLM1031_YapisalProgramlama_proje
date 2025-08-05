#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <time.h>
#include <locale.h>

void kes(char *s)
{
    while (*s)
    {
        if (*s == '\n' || *s == '\r')
        {
            *s = '\0';
        }
        s++;
    }
}

typedef struct
{
    int day;
    int month;
    int year;
} Date;

Date tarihiAyir(const char *date_str)
{
    Date date = {0};
    sscanf(date_str, "%d.%d.%d", &date.day, &date.month, &date.year);
    return date;
}

int tarihleriKarsilastir(Date d1, Date d2)
{
    if (d1.year != d2.year)
        return d1.year - d2.year;
    if (d1.month != d2.month)
        return d1.month - d2.month;
    return d1.day - d2.day;
}

int gecikmeHesaplama(Date d1, Date d2)
{
    struct tm tm1 = {0}, tm2 = {0};
    tm1.tm_year = d1.year - 1900;
    tm1.tm_mon = d1.month - 1;
    tm1.tm_mday = d1.day;

    tm2.tm_year = d2.year - 1900;
    tm2.tm_mon = d2.month - 1;
    tm2.tm_mday = d2.day;

    time_t time1 = mktime(&tm1);
    time_t time2 = mktime(&tm2);

    if (time1 == -1 || time2 == -1)
        return 0;

    return difftime(time1, time2) / (60 * 60 * 24);
}

typedef struct islem_arsivi
{
    char *etiket_no;
    int ogrenci_no;
    int islem_turu;
    char *tarih;
    struct islem_arsivi *next;
} islem_arsivi;

typedef struct ogrenci
{
    char *ad;
    char *soyad;
    int puan;
    int ogrenci_no;
    struct ogrenci *next;
    struct ogrenci *prev;
    islem_arsivi *islemler_head;
} ogrenci;

typedef struct yazar
{
    char *ad;
    char *soyad;
    int ID;
    struct yazar *next;
} yazar;

typedef struct kitap_ornek
{
    char *etiket_no;
    int rafta_mi;
    int ogrenci_no;
    struct kitap_ornek *next;
} kitap_ornek;

typedef struct kitap
{
    char *isbn;
    char *isim;
    int adet;
    kitap_ornek *ornekler;
    struct kitap *next;
} kitap;

typedef struct kitap_yazar
{
    char *isbn;
    int yazar_id;
} kitap_yazar;

typedef struct
{
    kitap_yazar *dizi;
    size_t count;
    size_t capacity;
} kitap_yazar_list;

typedef struct
{
    ogrenci *ogrenci_head;
    yazar *yazar_head;
    kitap *kitap_head;
    islem_arsivi *islem_head;
    kitap_yazar_list ky_list;
} kutuphane;

yazar *create_yazar(const char *ad, const char *soyad, int id);
ogrenci *create_ogrenci(const char *ad, const char *soyad, int no);
kitap *create_kitap(const char *isbn, const char *isim, int adet);
islem_arsivi *create_islem(const char *etiket_no, int ogrenci_no, int islem_turu, const char *tarih);
void free_kutuphane(kutuphane *ktp);
int add_kitap_yazar(kutuphane *ktp, const char *isbn, int yazar_id);
kitap *find_kitap_by_isbn(const kutuphane *ktp, const char *raw_isbn);

void bosluklariKes(char *s)
{
    char *start = s, *end;
    while (*start && isspace((unsigned char)*start))
        start++;
    if (start != s)
        memmove(s, start, strlen(start) + 1);
    end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end - 1)))
        *--end = '\0';
}

void free_list(kutuphane *ktp)
{
    for (size_t i = 0; i < ktp->ky_list.count; i++)
    {
        free(ktp->ky_list.dizi[i].isbn);
    }
    free(ktp->ky_list.dizi);
    ktp->ky_list.dizi = NULL;
    ktp->ky_list.count = ktp->ky_list.capacity = 0;
}

yazar *create_yazar(const char *ad, const char *soyad, int id)
{
    yazar *y = malloc(sizeof(yazar));
    y->ad = strdup(ad);
    y->soyad = strdup(soyad);
    y->ID = id;
    y->next = NULL;
    return y;
}

yazar *yazarSirala(yazar *head, yazar *new_yazar)
{
    if (!head || new_yazar->ID < head->ID)
    {
        new_yazar->next = head;
        return new_yazar;
    }
    yazar *current = head;
    while (current->next && current->next->ID < new_yazar->ID)
    {
        current = current->next;
    }
    new_yazar->next = current->next;
    current->next = new_yazar;
    return head;
}

int load_yazarlar(const char *dosya, kutuphane *ktp)
{
    FILE *fp = fopen(dosya, "r");
    if (!fp)
    {
        printf("Hata: %s dosyasi acilamadi.\n", dosya);
        return -1;
    }

    char line[256];
    if (fgets(line, sizeof(line), fp) == NULL)
    {
        printf("Hata: %s dosyasi bos.\n", dosya);
        fclose(fp);
        return -1;
    }

    while (fgets(line, sizeof(line), fp))
    {
        kes(line);
        char *id_str = strtok(line, ",");
        char *ad = strtok(NULL, ",");
        char *soyad = strtok(NULL, ",");

        if (ad && soyad && id_str)
        {
            bosluklariKes(ad);
            bosluklariKes(soyad);
            int id = atoi(id_str);
            yazar *y = create_yazar(ad, soyad, id);
            ktp->yazar_head = yazarSirala(ktp->yazar_head, y);
        }
        else
        {
            printf("Uyari: Eksik veri satiri: %s\n", line);
        }
    }

    fclose(fp);
    return 0;
}

int save_yazarlar(const char *dosya, kutuphane *ktp)
{
    FILE *fp = fopen(dosya, "w");
    if (!fp)
    {
        printf("Hata: %s dosyasi acilamadi.\n", dosya);
        return -1;
    }

    fprintf(fp, "ID,Ad,Soyad\n");
    for (yazar *cur = ktp->yazar_head; cur; cur = cur->next)
    {
        fprintf(fp, "%d,%s,%s\n", cur->ID, cur->ad, cur->soyad);
    }

    fclose(fp);
    return 0;
}

yazar *find_yazar_by_id(kutuphane *ktp, int id)
{
    for (yazar *cur = ktp->yazar_head; cur; cur = cur->next)
    {
        if (cur->ID == id)
            return cur;
    }
    return NULL;
}

int delete_yazar(kutuphane *ktp, int id)
{
    yazar *prev = NULL;
    yazar *cur = ktp->yazar_head;

    while (cur && cur->ID != id)
    {
        prev = cur;
        cur = cur->next;
    }

    if (!cur)
        return -1;

    if (prev)
    {
        prev->next = cur->next;
    }
    else
    {
        ktp->yazar_head = cur->next;
    }

    for (size_t i = 0; i < ktp->ky_list.count; i++)
    {
        if (ktp->ky_list.dizi[i].yazar_id == id)
        {
            ktp->ky_list.dizi[i].yazar_id = -1;
        }
    }

    free(cur->ad);
    free(cur->soyad);
    free(cur);
    return 0;
}

// 2. ogrenciler
ogrenci *create_ogrenci(const char *ad, const char *soyad, int no)
{
    ogrenci *o = malloc(sizeof(ogrenci));
    o->ad = strdup(ad);
    o->soyad = strdup(soyad);
    o->puan = 100;
    o->ogrenci_no = no;
    o->next = NULL;
    o->prev = NULL;
    o->islemler_head = NULL;
    return o;
}

int validate_student_no(int no)
{
    return no >= 10000000 && no <= 99999999;
}

int student_exists(kutuphane *ktp, int no)
{
    for (ogrenci *o = ktp->ogrenci_head; o; o = o->next)
    {
        if (o->ogrenci_no == no)
            return 1;
    }
    return 0;
}

int load_ogrenciler(const char *dosya, kutuphane *ktp)
{
    FILE *fp = fopen(dosya, "r");
    if (!fp)
        return -1;

    char line[256];
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp))
    {
        kes(line);
        char *ad = strtok(line, ",");
        char *soyad = strtok(NULL, ",");
        char *no_str = strtok(NULL, ",");
        char *puan_str = strtok(NULL, ",");

        if (ad && soyad && no_str && puan_str)
        {
            int no = atoi(no_str);
            int puan = atoi(puan_str);
            ogrenci *o = create_ogrenci(ad, soyad, no);
            o->puan = puan;

            o->next = ktp->ogrenci_head;
            if (ktp->ogrenci_head)
            {
                ktp->ogrenci_head->prev = o;
            }
            ktp->ogrenci_head = o;
        }
    }
    fclose(fp);
    return 0;
}
int save_ogrenciler(const char *dosya, kutuphane *ktp)
{
    FILE *fp = fopen(dosya, "w");
    if (!fp)
        return -1;

    fprintf(fp, "Ad,Soyad,OgrenciNo,Puan\n");
    for (ogrenci *cur = ktp->ogrenci_head; cur; cur = cur->next)
    {
        fprintf(fp, "%s,%s,%d,%d\n", cur->ad, cur->soyad, cur->ogrenci_no, cur->puan);
    }

    fclose(fp);
    return 0;
}

ogrenci *find_ogrenci_by_no(kutuphane *ktp, int no)
{
    for (ogrenci *cur = ktp->ogrenci_head; cur; cur = cur->next)
    {
        if (cur->ogrenci_no == no)
            return cur;
    }
    return NULL;
}

int ogrenciSil(kutuphane *ktp, int no)
{
    ogrenci *cur = ktp->ogrenci_head;

    while (cur && cur->ogrenci_no != no)
    {
        cur = cur->next;
    }

    if (!cur)
        return -1;

    for (kitap *k = ktp->kitap_head; k; k = k->next)
    {
        for (kitap_ornek *o = k->ornekler; o; o = o->next)
        {
            if (o->ogrenci_no == no)
            {
                o->rafta_mi = 1;
                o->ogrenci_no = -1;
            }
        }
    }

    if (cur->prev)
    {
        cur->prev->next = cur->next;
    }
    else
    {
        ktp->ogrenci_head = cur->next;
    }

    if (cur->next)
    {
        cur->next->prev = cur->prev;
    }

    free(cur->ad);
    free(cur->soyad);

    islem_arsivi *i = cur->islemler_head;
    while (i)
    {
        islem_arsivi *next = i->next;
        free(i->etiket_no);
        free(i->tarih);
        free(i);
        i = next;
    }

    free(cur);
    return 0;
}

// 3. Kitaplar
int isbn_kontrol(const char *isbn)
{
    if (strlen(isbn) != 13)
        return 0;
    for (int i = 0; i < 13; i++)
    {
        if (!isdigit(isbn[i]))
            return 0;
    }
    return 1;
}

kitap *create_kitap(const char *isbn, const char *isim, int adet)
{
    kitap *k = malloc(sizeof(kitap));
    k->isbn = strdup(isbn);
    k->isim = strdup(isim);
    k->adet = adet;
    k->next = NULL;
    k->ornekler = NULL;

    for (int i = 1; i <= adet; i++)
    {
        kitap_ornek *ornek = malloc(sizeof(kitap_ornek));
        char *label = malloc(strlen(isbn) + 10);
        sprintf(label, "%s_%d", isbn, i);
        ornek->etiket_no = label;
        ornek->rafta_mi = 1;
        ornek->ogrenci_no = -1;
        ornek->next = k->ornekler;
        k->ornekler = ornek;
    }

    return k;
}

int load_kitaplar(const char *dosya, kutuphane *ktp)
{
    FILE *fp = fopen(dosya, "r");
    if (!fp)
    {
        printf("Hata: %s dosyasi acilamadi.\n", dosya);
        return -1;
    }

    char line[512];
    if (fgets(line, sizeof(line), fp) == NULL)
    {
        printf("Hata: %s dosyasi bos.\n", dosya);
        fclose(fp);
        return -1;
    }

    while (fgets(line, sizeof(line), fp))
    {
        kes(line);
        char *isim = strtok(line, ",");
        char *isbn = strtok(NULL, ",");
        char *adet_str = strtok(NULL, ",");

        if (isbn && isim && adet_str)
        {
            bosluklariKes(isbn);
            bosluklariKes(isim);
            int adet = atoi(adet_str);

            if (!isbn_kontrol(isbn))
            {
                printf("Uyari: Gecersiz ISBN format: %s\n", isbn);
                continue;
            }

            if (find_kitap_by_isbn(ktp, isbn))
            {
                printf("Uyari: ISBN=%s olan kitap zaten mevcut.\n", isbn);
                continue;
            }

            kitap *k = create_kitap(isbn, isim, adet);
            k->next = ktp->kitap_head;
            ktp->kitap_head = k;
            printf("Kitap yuklendi: ISBN=%s, Isim=%s, Adet=%d\n", isbn, isim, adet);
        }
        else
        {
            printf("Uyari: Eksik veri satiri: %s\n", line);
        }
    }

    fclose(fp);
    return 0;
}

int save_kitaplar(const char *dosya, kutuphane *ktp)    
{
    FILE *fp = fopen(dosya, "w");
    if (!fp)
    {
        printf("Hata: %s dosyasi acilamadi.\n", dosya);
        return -1;
    }

    fprintf(fp, "Isim,ISBN,Adet\n");
    for (kitap *cur = ktp->kitap_head; cur; cur = cur->next)
    {
        fprintf(fp, "%s,%s,%d\n", 
            cur->isim,
            cur->isbn,
            cur->adet);
    }

    fclose(fp);
    return 0;
}

kitap *find_kitap_by_isbn(const kutuphane *ktp, const char *raw_isbn)
{
    if (!ktp || !raw_isbn)
        return NULL;
    char isbn[32];
    strncpy(isbn, raw_isbn, sizeof(isbn) - 1);
    isbn[sizeof(isbn) - 1] = '\0';
    kes(isbn);
    bosluklariKes(isbn);
    if (isbn[0] == '\0')
        return NULL;

    for (kitap *cur = ktp->kitap_head; cur; cur = cur->next)
    {
        if (cur->isbn && strcmp(cur->isbn, isbn) == 0)
            return cur;
    }
    return NULL;
}

kitap_ornek *find_kitap_ornek_by_label(kutuphane *ktp, const char *label)
{
    for (kitap *k = ktp->kitap_head; k; k = k->next)
    {
        for (kitap_ornek *o = k->ornekler; o; o = o->next)
        {
            if (strcmp(o->etiket_no, label) == 0)
                return o;
        }
    }
    return NULL;
}

int kitapSil(kutuphane *ktp, const char *isbn)
{
    kitap *prev = NULL;
    kitap *cur = ktp->kitap_head;

    while (cur && strcmp(cur->isbn, isbn) != 0)
    {
        prev = cur;
        cur = cur->next;
    }

    if (!cur)
        return -1;

    for (size_t i = 0; i < ktp->ky_list.count;)
    {
        if (strcmp(ktp->ky_list.dizi[i].isbn, isbn) == 0)
        {

            free(ktp->ky_list.dizi[i].isbn);
            for (size_t j = i; j < ktp->ky_list.count - 1; j++)
            {
                ktp->ky_list.dizi[j] = ktp->ky_list.dizi[j + 1];
            }
            ktp->ky_list.count--;
        }
        else
        {
            i++;
        }
    }

    if (prev)
    {
        prev->next = cur->next;
    }
    else
    {
        ktp->kitap_head = cur->next;
    }

    free(cur->isbn);
    free(cur->isim);

    kitap_ornek *o = cur->ornekler;
    while (o)
    {
    tag:
        kitap_ornek *next_o = o->next;
        free(o->etiket_no);
        free(o);
        o = next_o;
    }

    free(cur);
    return 0;
}

// 4. Kitap ornekleri
int load_kitap_ornekleri(const char *dosya, kutuphane *ktp)
{
    FILE *fp = fopen(dosya, "r");
    if (!fp)
    {
        printf("Hata: %s dosyasi acilamadi.\n", dosya);
        return -1;
    }

    char line[256];
    if (fgets(line, sizeof(line), fp) == NULL)
    {
        printf("Uyari: %s dosyasi bos.\n", dosya);
        fclose(fp);
        return 0;
    }

    while (fgets(line, sizeof(line), fp))
    {
        kes(line);
        char *etiket = strtok(line, ",");
        char *isbn = strtok(NULL, ",");
        char *rafta_str = strtok(NULL, ",");
        char *no_str = strtok(NULL, ",");

        if (etiket && isbn && rafta_str && no_str)
        {
            bosluklariKes(isbn);
            int rafta = atoi(rafta_str);
            int o_no = atoi(no_str);

            // Find the book by ISBN
            kitap *k = NULL;
            for (kitap *cur = ktp->kitap_head; cur; cur = cur->next)
            {
                if (strcmp(cur->isbn, isbn) == 0)
                {
                    k = cur;
                    break;
                }
            }

            if (k)
            {
                // Check if this instance already exists
                kitap_ornek *existing = NULL;
                for (kitap_ornek *o = k->ornekler; o; o = o->next)
                {
                    if (strcmp(o->etiket_no, etiket) == 0)
                    {
                        existing = o;
                        break;
                    }
                }

                if (!existing)
                {
                    // Create new instance
                    kitap_ornek *o = malloc(sizeof(kitap_ornek));
                    o->etiket_no = strdup(etiket);
                    o->rafta_mi = rafta;
                    o->ogrenci_no = o_no;
                    o->next = k->ornekler;
                    k->ornekler = o;
                }
                else
                {
                    // Update existing instance
                    existing->rafta_mi = rafta;
                    existing->ogrenci_no = o_no;
                }
            }
            else
            {
                printf("Uyari: ISBN=%s olan kitap bulunamadi.\n", isbn);
            }
        }
    }

    fclose(fp);
    return 0;
}

int save_kitap_ornekleri(const char *dosya, kutuphane *ktp)
{
    FILE *fp = fopen(dosya, "w");
    if (!fp)
    {
        printf("Hata: %s dosyasi acilamadi.\n", dosya);
        return -1;
    }

    fprintf(fp, "EtiketNo,ISBN,RaftaMi,OgrenciNo\n");
    for (kitap *k = ktp->kitap_head; k; k = k->next)
    {
        for (kitap_ornek *o = k->ornekler; o; o = o->next)
        {
            fprintf(fp, "%s,%s,%d,%d\n", 
                o->etiket_no,
                k->isbn,
                o->rafta_mi,
                o->ogrenci_no);
        }
    }

    fclose(fp);
    return 0;
}

// 5. Kitap–Yazar Eslestirme
int load_kitap_yazar(const char *dosya, kutuphane *ktp)
{
    if (!dosya || !ktp)
    {
        printf("Hata: Gecersiz parametreler.\n");
        return -1;
    }

    FILE *fp = fopen(dosya, "r");
    if (!fp)
    {
        printf("Hata: Dosya acilamadi: %s\n", dosya);
        return -1;
    }

    char line[512];
    if (fgets(line, sizeof(line), fp) == NULL)
    {
        printf("Uyari: Dosya bos veya baslik okunamadi.\n");
        fclose(fp);
        return 0;
    }

    ktp->ky_list.count = 0;
    ktp->ky_list.capacity = 16;
    ktp->ky_list.dizi = malloc(ktp->ky_list.capacity * sizeof(kitap_yazar));
    if (!ktp->ky_list.dizi)
    {
        printf("Hata: Bellek ayirma basarisiz.\n");
        fclose(fp);
        return -1;
    }

    while (fgets(line, sizeof(line), fp))
    {
        kes(line);
        bosluklariKes(line);

        char *isbn = strtok(line, ",");
        char *id_str = strtok(NULL, ",");

        if (isbn && id_str)
        {
            bosluklariKes(isbn);
            bosluklariKes(id_str);

            if (ktp->ky_list.count == ktp->ky_list.capacity)
            {
                size_t newcap = ktp->ky_list.capacity * 2;
                kitap_yazar *tmp = realloc(ktp->ky_list.dizi,
                                           newcap * sizeof(kitap_yazar));
                if (!tmp)
                {
                    printf("Hata: Bellek yeniden ayirma basarisiz.\n");
                    free_list(ktp);
                    fclose(fp);
                    return -1;
                }
                ktp->ky_list.dizi = tmp;
                ktp->ky_list.capacity = newcap;
            }

            kitap_yazar *entry = &ktp->ky_list.dizi[ktp->ky_list.count];
            entry->isbn = strdup(isbn);
            if (!entry->isbn)
            {
                printf("Hata: ISBN kopyalama basarisiz.\n");
                free_list(ktp);
                fclose(fp);
                return -1;
            }
            entry->yazar_id = atoi(id_str);
            ktp->ky_list.count++;
        }
        else
        {
            printf("Uyari: Eksik veri satiri: %s\n", line);
        }
    }

    fclose(fp);
    return 0;
}

int save_kitap_yazar(const char *dosya, kutuphane *ktp)
{
    FILE *fp = fopen(dosya, "w");
    if (!fp)
        return -1;

    fprintf(fp, "ISBN,YazarID\n");
    for (size_t i = 0; i < ktp->ky_list.count; ++i)
    {
        fprintf(fp, "%s,%d\n", ktp->ky_list.dizi[i].isbn, ktp->ky_list.dizi[i].yazar_id);
    }

    fclose(fp);
    return 0;
}

int add_kitap_yazar(kutuphane *ktp, const char *isbn, int yazar_id)
{
    if (!find_kitap_by_isbn(ktp, isbn))
        return -1;

    if (yazar_id != -1 && !find_yazar_by_id(ktp, yazar_id))
        return -2;

    for (size_t i = 0; i < ktp->ky_list.count; i++)
    {
        if (strcmp(ktp->ky_list.dizi[i].isbn, isbn) == 0 &&
            ktp->ky_list.dizi[i].yazar_id == yazar_id)
        {
            return 0;
        }
    }

    if (ktp->ky_list.count == ktp->ky_list.capacity)
    {
        size_t new_cap = ktp->ky_list.capacity * 2;
        kitap_yazar *new_dizi = realloc(ktp->ky_list.dizi, new_cap * sizeof(kitap_yazar));
        if (!new_dizi)
            return -3;
        ktp->ky_list.dizi = new_dizi;
        ktp->ky_list.capacity = new_cap;
    }

    kitap_yazar *e = &ktp->ky_list.dizi[ktp->ky_list.count++];
    e->isbn = strdup(isbn);
    e->yazar_id = yazar_id;

    return 1;
}

int remove_kitap_yazar(kutuphane *ktp, const char *isbn, int yazar_id)
{
    for (size_t i = 0; i < ktp->ky_list.count; i++)
    {
        if (strcmp(ktp->ky_list.dizi[i].isbn, isbn) == 0 &&
            ktp->ky_list.dizi[i].yazar_id == yazar_id)
        {
            free(ktp->ky_list.dizi[i].isbn);
            for (size_t j = i; j < ktp->ky_list.count - 1; j++)
            {
                ktp->ky_list.dizi[j] = ktp->ky_list.dizi[j + 1];
            }
            ktp->ky_list.count--;
            return 1;
        }
    }
    return 0;
}

// 6. Islemler
islem_arsivi *create_islem(const char *etiket_no, int ogrenci_no, int islem_turu, const char *tarih)
{
    islem_arsivi *i = malloc(sizeof(islem_arsivi));
    i->etiket_no = strdup(etiket_no);
    i->ogrenci_no = ogrenci_no;
    i->islem_turu = islem_turu;
    i->tarih = strdup(tarih);
    i->next = NULL;
    return i;
}

int load_islemler(const char *dosya, kutuphane *ktp)
{
    FILE *fp = fopen(dosya, "r");
    if (!fp)
        return -1;

    char line[256];

    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp))
    {
        kes(line);
        char *etiket = strtok(line, ",");
        char *no_str = strtok(NULL, ",");
        char *tur_str = strtok(NULL, ",");
        char *tarih = strtok(NULL, ",");

        if (etiket && no_str && tur_str && tarih)
        {
            int ogrenci_no = atoi(no_str);
            int islem_turu = atoi(tur_str);

            // Create transaction record for library
            islem_arsivi *i = create_islem(etiket, ogrenci_no, islem_turu, tarih);
            i->next = ktp->islem_head;
            ktp->islem_head = i;

            // Create separate transaction record for student
            ogrenci *o = find_ogrenci_by_no(ktp, ogrenci_no);
            if (o)
            {
                islem_arsivi *student_islem = create_islem(etiket, ogrenci_no, islem_turu, tarih);
                student_islem->next = o->islemler_head;
                o->islemler_head = student_islem;
            }
        }
    }

    fclose(fp);
    return 0;
}

int save_islemler(const char *dosya, kutuphane *ktp)
{
    FILE *fp = fopen(dosya, "w");
    if (!fp)
        return -1;

    fprintf(fp, "EtiketNo,OgrenciNo,IslemTuru,Tarih\n");
    for (islem_arsivi *cur = ktp->islem_head; cur; cur = cur->next)
    {
        fprintf(fp, "%s,%d,%d,%s\n", cur->etiket_no, cur->ogrenci_no, cur->islem_turu, cur->tarih);
    }

    fclose(fp);
    return 0;
}

// 7. Borrow/Return operations
int borrow_book(kutuphane *ktp, const char *isbn, int ogrenci_no, const char *tarih)
{
    ogrenci *o = find_ogrenci_by_no(ktp, ogrenci_no);
    if (!o)
        return -1;
    if (o->puan < 0)
        return -2;

    kitap *k = find_kitap_by_isbn(ktp, isbn);
    if (!k)
        return -3;

    kitap_ornek *available = k->ornekler;
    while (available && !available->rafta_mi)
        available = available->next;

    if (!available)
        return -4;

    available->rafta_mi = 0;
    available->ogrenci_no = ogrenci_no;

    islem_arsivi *i = create_islem(available->etiket_no, ogrenci_no, 0, tarih);

    // Add to kutuphane's islem list
    i->next = ktp->islem_head;
    ktp->islem_head = i;

    // Add to student's islem list
    islem_arsivi *student_islem = create_islem(available->etiket_no, ogrenci_no, 0, tarih);
    student_islem->next = o->islemler_head;
    o->islemler_head = student_islem;

    return 0;
}

int return_book(kutuphane *ktp, const char *etiket_no, int ogrenci_no, const char *tarih)
{
    kitap_ornek *o = find_kitap_ornek_by_label(ktp, etiket_no);
    if (!o)
    {
        printf("Hata: Etiket numarasi bulunamadi: %s\n", etiket_no);
        return -1;
    }

    if (o->rafta_mi)
    {
        printf("Hata: Kitap zaten rafta.\n");
        return -2;
    }

    if (o->ogrenci_no != ogrenci_no)
    {
        printf("Hata: Kitap bu ogrenciye ait degil.\n");
        return -3;
    }

    ogrenci *student = find_ogrenci_by_no(ktp, ogrenci_no);
    if (!student)
    {
        printf("Hata: Ogrenci bulunamadi.\n");
        return -4;
    }

    // Find the borrow transaction
    islem_arsivi *borrow_trans = NULL;
    for (islem_arsivi *i = student->islemler_head; i; i = i->next)
    {
        if (strcmp(i->etiket_no, etiket_no) == 0 && i->islem_turu == 0)
        {
            borrow_trans = i;
            break;
        }
    }

    if (!borrow_trans)
    {
        printf("Hata: Odunc alma kaydi bulunamadi.\n");
        return -5;
    }

    // Check for late return
    Date borrow_date = tarihiAyir(borrow_trans->tarih);
    Date return_date = tarihiAyir(tarih);
    int days = gecikmeHesaplama(return_date, borrow_date);

    if (days > 15)
    {
        student->puan -= 10;
        printf("Uyari: Kitap %d gun gecikmeli teslim edildi. 10 puan dusuldu.\n", days);
    }

    // Update book status
    o->rafta_mi = 1;
    o->ogrenci_no = -1;

    // Add return transaction to kutuphane's islem list
    islem_arsivi *return_trans = create_islem(etiket_no, ogrenci_no, 1, tarih);
    return_trans->next = ktp->islem_head;
    ktp->islem_head = return_trans;

    // Add return transaction to student's islem list
    islem_arsivi *student_return = create_islem(etiket_no, ogrenci_no, 1, tarih);
    student_return->next = student->islemler_head;
    student->islemler_head = student_return;

    return 0;
}

// 8. List functions
void list_students_with_penalty(kutuphane *ktp)
{
    printf("\n--- Cezali ogrenciler ---\n");
    int found = 0;
    for (ogrenci *o = ktp->ogrenci_head; o; o = o->next)
    {
        if (o->puan < 0)
        {
            printf("ogrenci No: %d, Adi: %s, Soyadi: %s, Puan: %d\n",
                   o->ogrenci_no, o->ad, o->soyad, o->puan);
            found = 1;
        }
    }
    if (!found)
    {
        printf("Cezali ogrenci bulunmamaktadir.\n");
    }
}

void list_students_with_overdue_books(kutuphane *ktp, const char *current_date)
{
    printf("\n--- Kitap Teslim Etmemis ogrenciler ---\n");
    Date today = tarihiAyir(current_date);
    int found_overdue_student = 0;

    ogrenci *o = ktp->ogrenci_head;
    while (o)
    {
        int has_overdue = 0;
        islem_arsivi *i = o->islemler_head;
        while (i && !has_overdue)
        {
            if (i->islem_turu == 0)
            {

                int returned = 0;
                islem_arsivi *j = o->islemler_head;
                while (j && !(j->islem_turu == 1 && strcmp(j->etiket_no, i->etiket_no) == 0))
                {
                    j = j->next;
                }
                if (j)
                    returned = 1;

                if (!returned)
                {
                    Date borrow_date = tarihiAyir(i->tarih);
                    int days = gecikmeHesaplama(today, borrow_date);
                    if (days > 15)
                    {
                        has_overdue = 1;
                    }
                }
            }
            if (!has_overdue)
                i = i->next;
        }

        if (has_overdue)
        {
            printf("ogrenci No: %d, Adi: %s, Soyadi: %s\n",
                   o->ogrenci_no, o->ad, o->soyad);
            found_overdue_student = 1;
        }

        o = o->next;
    }

    if (!found_overdue_student)
    {
        printf("Teslim etmemis ogrenci bulunmamaktadir.\n");
    }
}

void ogrencileriListele(kutuphane *ktp)
{
    printf("\n--- Tum ogrenciler ---\n");
    if (!ktp->ogrenci_head)
    {
        printf("Sistemde kayitli ogrenci bulunmamaktadir.\n");
        return;
    }
    for (ogrenci *cur = ktp->ogrenci_head; cur; cur = cur->next)
    {
        printf("ogrenci No: %d, Adi: %s, Soyadi: %s, Puan: %d\n", cur->ogrenci_no, cur->ad, cur->soyad, cur->puan);
    }
}

void raftakiKitaplariListele(kutuphane *ktp)
{
    printf("\n--- Raftaki Kitaplar ---\n");
    int found_book = 0;
    for (kitap *k = ktp->kitap_head; k; k = k->next)
    {
        int available_copies = 0;
        for (kitap_ornek *o = k->ornekler; o; o = o->next)
        {
            if (o->rafta_mi)
            {
                available_copies++;
            }
        }
        if (available_copies > 0)
        {
            printf("Kitap Adi: %s, ISBN: %s, Raftaki Adet: %d\n",
                   k->isim,
                   k->isbn,
                   available_copies);
            found_book = 1;
        }
    }
    if (!found_book)
    {
        printf("Rafta kitap bulunmamaktadir.\n");
    }
}

void gecikmisKitapListele(kutuphane *ktp, const char *current_date)
{
    printf("\n--- Zamaninda Teslim Edilmeyen Kitaplar ---\n");
    Date today = tarihiAyir(current_date);
    int found_overdue_book = 0;

    for (kitap *k = ktp->kitap_head; k; k = k->next)
    {
        for (kitap_ornek *instance = k->ornekler; instance; instance = instance->next)
        {
            if (!instance->rafta_mi && instance->ogrenci_no != -1)
            {
                islem_arsivi *latest_borrow = NULL;
                ogrenci *borrowing_student = find_ogrenci_by_no(ktp, instance->ogrenci_no);

                if (borrowing_student)
                {
                    for (islem_arsivi *trans = borrowing_student->islemler_head; trans; trans = trans->next)
                    {
                        if (strcmp(trans->etiket_no, instance->etiket_no) == 0 && trans->islem_turu == 0) // Borrow transaction
                        {
                            if (!latest_borrow || tarihleriKarsilastir(tarihiAyir(trans->tarih), tarihiAyir(latest_borrow->tarih)) > 0)
                            {
                                latest_borrow = trans;
                            }
                        }
                    }
                }

                if (latest_borrow)
                {
                    int returned = 0;
                    if (borrowing_student)
                    {
                        for (islem_arsivi *trans = borrowing_student->islemler_head; trans; trans = trans->next)
                        {
                            if (strcmp(trans->etiket_no, instance->etiket_no) == 0 && trans->islem_turu == 1 && tarihleriKarsilastir(tarihiAyir(trans->tarih), tarihiAyir(latest_borrow->tarih)) > 0)
                            {
                                returned = 1;
                                break;
                            }
                        }
                    }

                    if (!returned)
                    {
                        Date borrow_date = tarihiAyir(latest_borrow->tarih);
                        int days = gecikmeHesaplama(today, borrow_date);
                        if (days > 15)
                        {
                            printf("Kitap Adi: %s, Etiket No: %s, ogrenci No: %d, odunc Tarihi: %s\n",
                                   k->isim, instance->etiket_no, instance->ogrenci_no, latest_borrow->tarih);
                            found_overdue_book = 1;
                        }
                    }
                }
            }
        }
    }
    if (!found_overdue_book)
    {
        printf("Zamaninda teslim edilmeyen kitap bulunmamaktadir.\n");
    }
}

// 9. Memory management
void free_kutuphane(kutuphane *ktp)
{
    yazar *y = ktp->yazar_head;
    while (y)
    {
        yazar *next = y->next;
        free(y->ad);
        free(y->soyad);
        free(y);
        y = next;
    }

    ogrenci *o = ktp->ogrenci_head;
    while (o)
    {
        ogrenci *next = o->next;
        free(o->ad);
        free(o->soyad);

        islem_arsivi *i = o->islemler_head;
        while (i)
        {
            islem_arsivi *next_i = i->next;
            free(i->etiket_no);
            free(i->tarih);
            free(i);
            i = next_i;
        }

        free(o);
        o = next;
    }

    // Free books
    kitap *k = ktp->kitap_head;
    while (k)
    {
        kitap *next = k->next;
        free(k->isbn);
        free(k->isim);

        kitap_ornek *ornek = k->ornekler;
        while (ornek)
        {
            kitap_ornek *next_ornek = ornek->next;
            free(ornek->etiket_no);
            free(ornek);
            ornek = next_ornek;
        }

        free(k);
        k = next;
    }

    // Free book-author relationships
    for (size_t i = 0; i < ktp->ky_list.count; i++)
    {
        free(ktp->ky_list.dizi[i].isbn);
    }
    free(ktp->ky_list.dizi);

    free(ktp);
}

void get_current_date_str(char *date_str)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(date_str, "%02d.%02d.%d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
}


void printOgrenci(kutuphane *ktp, int ogrenci_no)
{
    ogrenci *s = find_ogrenci_by_no(ktp, ogrenci_no);
    if (!s)
    {
        printf("ogrenci bulunamadi.\n");
        return;
    }

    printf("\n--- ogrenci Bilgileri ---\n");
    printf("Ad: %s\n", s->ad);
    printf("Soyad: %s\n", s->soyad);    
    printf("ogrenci No: %d\n", s->ogrenci_no);
    printf("Puan: %d\n", s->puan);

    printf("\n--- Kitap Hareketleri ---\n");
    

    int found_transactions = 0;
    for (islem_arsivi *i = s->islemler_head; i; i = i->next)
    {
        if (!found_transactions)
        {
            found_transactions = 1;
        }
        printf("Etiket No: %s, Islem Turu: %s, Tarih: %s\n",
               i->etiket_no, 
               (i->islem_turu == 0 ? "odunc Alma" : "Teslim Etme"), 
               i->tarih);
    }

    if (!found_transactions)
    {
        printf("Bu ogrenciye ait kitap hareketi bulunmamaktadir.\n");
    }
}

void printKitap(kutuphane *ktp, const char *search_term)
{
    printf("\n--- Kitap Bilgileri ---\n");
    int found_book = 0;
    char search_lower[256];
    strcpy(search_lower, search_term);
    // Convert search string to lowercase
    for(int i = 0; search_lower[i]; i++) {
        search_lower[i] = tolower(search_lower[i]);
    }

    // First try to find by ISBN
    kitap *k = find_kitap_by_isbn(ktp, search_term);
    if (k)
    {
        printf("Kitap Adi: %s\n", k->isim);
        printf("ISBN: %s\n", k->isbn);
        printf("Toplam Adet: %d\n", k->adet);
        found_book = 1;

        printf("\n--- Kitap ornekleri ---\n");
        if (!k->ornekler)
        {
            printf("Bu kitaba ait ornek bulunmamaktadir.\n");
        }
        else
        {
            for (kitap_ornek *o = k->ornekler; o; o = o->next)
            {
                printf("Etiket No: %s, Durum: %s", o->etiket_no, (o->rafta_mi ? "Rafta" : "oduncte"));
                if (!o->rafta_mi)
                {
                    printf(", odunc Alan ogrenci No: %d", o->ogrenci_no);
                }
                printf("\n");
            }
        }
        return;
    }

    // If not found by ISBN, try to find by name
    for (kitap *k = ktp->kitap_head; k; k = k->next)
    {
        char book_name_lower[256];
        strcpy(book_name_lower, k->isim);
        // Convert book name to lowercase
        for(int i = 0; book_name_lower[i]; i++) {
            book_name_lower[i] = tolower(book_name_lower[i]);
        }

        if (strstr(book_name_lower, search_lower) != NULL)
        {
            printf("Kitap Adi: %s\n", k->isim);
            printf("ISBN: %s\n", k->isbn);
            printf("Toplam Adet: %d\n", k->adet);
            found_book = 1;

            printf("\n--- Kitap ornekleri ---\n");
            if (!k->ornekler)
            {
                printf("Bu kitaba ait ornek bulunmamaktadir.\n");
            }
            else
            {
                for (kitap_ornek *o = k->ornekler; o; o = o->next)
                {
                    printf("Etiket No: %s, Durum: %s", o->etiket_no, (o->rafta_mi ? "Rafta" : "oduncte"));
                    if (!o->rafta_mi)
                    {
                        printf(", odunc Alan ogrenci No: %d", o->ogrenci_no);
                    }
                    printf("\n");
                }
            }
            return;
        }
    }

    if (!found_book)
    {
        printf("Kitap bulunamadi: %s\n", search_term);
        printf("\nMevcut Kitaplar:\n");
        printf("--------------------------------\n");
        for (kitap *k = ktp->kitap_head; k; k = k->next)
        {
            printf("Kitap Adi: %s, ISBN: %s\n", k->isim, k->isbn);
        }
        printf("--------------------------------\n");
    }
}
void printYazar(kutuphane *ktp, const char *authorID)
{
    printf("\n--- Yazar Bilgileri ---\n");
    int author_id_int = atoi(authorID);

    yazar *found_author = ktp->yazar_head;
    while (found_author && found_author->ID != author_id_int)
        found_author = found_author->next;

    if (!found_author)
    {
        printf("Yazar bulunamadi: %s\n", authorID);
        return;
    }

    printf("Ad: %s, Soyad: %s, ID: %d\n",
           found_author->ad, found_author->soyad, found_author->ID);

    printf("\n--- Yazara Ait Kitaplar ---\n");
    int found_book_for_author = 0;
    for (size_t i = 0; i < ktp->ky_list.count; ++i)
    {
        if (ktp->ky_list.dizi[i].yazar_id == found_author->ID)
        {
            kitap *k = find_kitap_by_isbn(ktp, ktp->ky_list.dizi[i].isbn);
            if (k)
            {
                printf("  Kitap Adi: %s, ISBN: %s\n",
                       k->isim, k->isbn);
                found_book_for_author = 1;
            }
        }
    }
    if (!found_book_for_author)
    {
        printf("Bu yazara ait kitap bulunmamaktadir.\n");
    }
}

void yazarEKle(kutuphane *ktp)
{
    char ad[100], soyad[100];
    int id;

    printf("Yazar Adi: ");
    fgets(ad, sizeof(ad), stdin);
    kes(ad);
    bosluklariKes(ad);

    printf("Yazar Soyadi: ");
    fgets(soyad, sizeof(soyad), stdin);
    kes(soyad);
    bosluklariKes(soyad);

    int max_id = 0;
    for (yazar *cur = ktp->yazar_head; cur; cur = cur->next)
    {
        if (cur->ID > max_id)
        {
            max_id = cur->ID;
        }
    }
    id = max_id + 1;

    yazar *new_yazar = create_yazar(ad, soyad, id);
    ktp->yazar_head = yazarSirala(ktp->yazar_head, new_yazar);
    
    if (save_yazarlar("Yazarlar.csv", ktp) != 0)
    {
        printf("Hata: Yazarlar.csv dosyasina kaydedilemedi.\n");
        return;
    }
    printf("Yazar basariyla eklendi (ID: %d).\n", id);
}

void update_author(kutuphane *ktp)
{
    int id;
    char new_ad[100], new_soyad[100];

    printf("Guncellenecek yazar ID'si: ");
    scanf("%d", &id);
    while (getchar() != '\n')
        ;

    yazar *y = find_yazar_by_id(ktp, id);
    if (!y)
    {
        printf("Yazar bulunamadi.\n");
        return;
    }

    printf("Yeni ad (mevcut: %s): ", y->ad);
    fgets(new_ad, sizeof(new_ad), stdin);
    kes(new_ad);
    bosluklariKes(new_ad);
    free(y->ad);
    y->ad = strdup(new_ad);

    printf("Yeni soyad (mevcut: %s): ", y->soyad);
    fgets(new_soyad, sizeof(new_soyad), stdin);
    kes(new_soyad);
    bosluklariKes(new_soyad);
    free(y->soyad);
    y->soyad = strdup(new_soyad);

    if (save_yazarlar("Yazarlar.csv", ktp) != 0)
    {
        printf("Hata: Yazarlar.csv dosyasina kaydedilemedi.\n");
        return;
    }
    printf("Yazar bilgileri basariyla guncellendi.\n");
}

void ogrenciEkle(kutuphane *ktp)
{
    char ad[100], soyad[100];
    int ogrenci_no;

    printf("ogrenci Adi: ");
    fgets(ad, sizeof(ad), stdin);
    kes(ad);

    printf("ogrenci Soyadi: ");
    fgets(soyad, sizeof(soyad), stdin);
    kes(soyad);

    printf("ogrenci Numarasi (8 haneli): ");
    scanf("%d", &ogrenci_no);
    while (getchar() != '\n')
        ;

    if (!validate_student_no(ogrenci_no))
    {
        printf("Gecersiz ogrenci numarasi. 8 haneli olmalidir.\n");
        return;
    }

    if (student_exists(ktp, ogrenci_no))
    {
        printf("Bu ogrenci numarasi zaten mevcut.\n");
        return;
    }

    ogrenci *new_ogrenci = create_ogrenci(ad, soyad, ogrenci_no);
    new_ogrenci->next = ktp->ogrenci_head;
    if (ktp->ogrenci_head)
    {
        ktp->ogrenci_head->prev = new_ogrenci;
    }
    ktp->ogrenci_head = new_ogrenci;

    save_ogrenciler("ogrenciler.csv", ktp);
    printf("ogrenci basariyla eklendi.\n");
}

void ogrenciGuncelle(kutuphane *ktp)
{
    int ogrenci_no;
    char new_ad[100], new_soyad[100];
    int new_puan;

    printf("Guncellenecek ogrenci numarasi: ");
    scanf("%d", &ogrenci_no);
    while (getchar() != '\n')
        ; // Clear buffer

    ogrenci *s = find_ogrenci_by_no(ktp, ogrenci_no);
    if (!s)
    {
        printf("ogrenci bulunamadi.\n");
        return;
    }

    printf("Yeni ad (mevcut: %s): ", s->ad);
    fgets(new_ad, sizeof(new_ad), stdin);
    kes(new_ad);
    free(s->ad);
    s->ad = strdup(new_ad);

    printf("Yeni soyad (mevcut: %s): ", s->soyad);
    fgets(new_soyad, sizeof(new_soyad), stdin);
    kes(new_soyad);
    free(s->soyad);
    s->soyad = strdup(new_soyad);

    printf("Yeni puan (mevcut: %d): ", s->puan);
    scanf("%d", &new_puan);
    while (getchar() != '\n')
        ;
    s->puan = new_puan;

    save_ogrenciler("ogrenciler.csv", ktp);
    printf("ogrenci bilgileri basariyla guncellendi.\n");
}

void kitapEkle(kutuphane *ktp)
{
    char isbn[15], isim[256];
    int adet;

    printf("Kitap Adi: ");
    fgets(isim, sizeof(isim), stdin);
    kes(isim);

    printf("ISBN (13 Haneli): ");
    fgets(isbn, sizeof(isbn), stdin);
    kes(isbn);

    if (!isbn_kontrol(isbn))
    {
        printf("Gecersiz ISBN. 13 haneli sayi olmalidir.\n");
        return;
    }

    if (find_kitap_by_isbn(ktp, isbn))
    {
        printf("Bu ISBN numarasina sahip bir kitap zaten mevcut.\n");
        return;
    }

    printf("Adet: ");
    scanf("%d", &adet);
    while (getchar() != '\n')
        ;

    kitap *new_kitap = create_kitap(isbn, isim, adet);
    new_kitap->next = ktp->kitap_head;
    ktp->kitap_head = new_kitap;

    save_kitaplar("kitaplar.csv", ktp);
    save_kitap_ornekleri("kitap_ornekleri.csv", ktp);
    printf("Kitap basariyla eklendi.\n");
}

void kitapGuncelle(kutuphane *ktp)
{
    char isbn[15], new_isim[256];
    int new_adet;

    printf("Guncellenecek kitabin ISBN'i: ");
    fgets(isbn, sizeof(isbn), stdin);
    kes(isbn);

    kitap *k = find_kitap_by_isbn(ktp, isbn);
    if (!k)
    {
        printf("Kitap bulunamadi.\n");
        return;
    }

    printf("Yeni kitap adi (mevcut: %s): ", k->isim);
    fgets(new_isim, sizeof(new_isim), stdin);
    kes(new_isim);
    free(k->isim);
    k->isim = strdup(new_isim);

    printf("Yeni adet (mevcut: %d): ", k->adet);
    scanf("%d", &new_adet);
    while (getchar() != '\n')
        ;

    if (new_adet < k->adet)
    {
        int to_remove = k->adet - new_adet;
        int can_remove = 1;
        while (to_remove > 0 && k->ornekler && can_remove)
        {
            kitap_ornek *to_delete = k->ornekler;
            k->ornekler = to_delete->next;

            if (!to_delete->rafta_mi)
            {
                printf("Uyari: Silinecek kopya oduncte! Islem iptal edildi.\n");
                k->ornekler = to_delete;
                can_remove = 0;
            }
            else
            {
                free(to_delete->etiket_no);
                free(to_delete);
                to_remove--;
                k->adet--;
            }
        }
    }
    else if (new_adet > k->adet)
    {
        int to_add = new_adet - k->adet;
        for (int i = 0; i < to_add; i++)
        {
            kitap_ornek *new_copy = malloc(sizeof(kitap_ornek));
            char label[50];
            sprintf(label, "%s_%d", k->isbn, k->adet + i + 1);
            new_copy->etiket_no = strdup(label);
            new_copy->rafta_mi = 1;
            new_copy->ogrenci_no = -1;
            new_copy->next = k->ornekler;
            k->ornekler = new_copy;
        }
        k->adet = new_adet;
    }

    save_kitaplar("kitaplar.csv", ktp);
    save_kitap_ornekleri("kitap_ornekleri.csv", ktp);
    printf("Kitap bilgileri basariyla guncellendi.\n");
}

void match_book_author(kutuphane *ktp)
{
    char isbn_buf[32];
    int yazar_id;
    const char *csv_file = "KitapYazar.csv";

    // ISBN oku
    printf("Eslestirilecek kitabin ISBN'i: ");
    if (!fgets(isbn_buf, sizeof(isbn_buf), stdin))
    {
        printf("Hata: ISBN okunamadi.\n");
        return;
    }
    kes(isbn_buf);
    bosluklariKes(isbn_buf);
    if (isbn_buf[0] == '\0')
    {
        printf("Hata: Bos ISBN girildi.\n");
        return;
    }

    printf("Eslestirilecek yazarin ID'si: ");
    if (scanf("%d", &yazar_id) != 1)
    {
        printf("Hata: Gecersiz ID formati.\n");
        while (getchar() != '\n')
            ;
        return;
    }
    while (getchar() != '\n')
        ;

    kitap *k = find_kitap_by_isbn(ktp, isbn_buf);
    if (!k)
    {
        printf("Hata: Kitap bulunamadi (ISBN=\"%s\").\n", isbn_buf);
        return;
    }

    yazar *y = find_yazar_by_id(ktp, yazar_id);
    if (!y)
    {
        printf("Hata: Yazar bulunamadi (ID=%d).\n", yazar_id);
        return;
    }

    int res = add_kitap_yazar(ktp, isbn_buf, yazar_id);
    if (res > 0)
    {
        if (save_kitap_yazar(csv_file, ktp) != 0)
        {
            perror("save_kitap_yazar");
        }

        FILE *fp = fopen(csv_file, "a");
        if (!fp)
        {
            perror(csv_file);
        }
        else
        {
            fprintf(fp, "%s,%d\n", isbn_buf, yazar_id);
            fclose(fp);
        }

        printf("Basari: Kitap (ISBN=%s) ile Yazar (ID=%d) eslestirildi ve \"%s\" dosyasina kaydedildi.\n",
               isbn_buf, yazar_id, csv_file);
    }
    else if (res == 0)
    {
        printf("Uyari: Bu eslestirme zaten mevcut.\n");
    }
    else
    {
        printf("Hata: Eslestirme sirasinda hata kodu %d dondu.\n", res);
    }
}

void kitapyazariniGuncelle(kutuphane *ktp)
{
    char isbn[15];
    int old_yazar_id, new_yazar_id;

    printf("Yazari guncellenecek kitabin ISBN'i: ");
    fgets(isbn, sizeof(isbn), stdin);
    kes(isbn);
    bosluklariKes(isbn);

    if (!isbn_kontrol(isbn))
    {
        printf("Hata: Gecersiz ISBN format. 13 haneli sayi olmalidir.\n");
        return;
    }

    kitap *k = find_kitap_by_isbn(ktp, isbn);
    if (!k)
    {
        printf("Hata: Kitap bulunamadi (ISBN=%s).\n", isbn);
        return;
    }

    printf("Mevcut yazar ID'si: ");
    scanf("%d", &old_yazar_id);
    while (getchar() != '\n')
        ;

    printf("Yeni yazar ID'si (-1 silmek icin): ");
    scanf("%d", &new_yazar_id);
    while (getchar() != '\n')
        ;

    if (old_yazar_id != -1 && !find_yazar_by_id(ktp, old_yazar_id))
    {
        printf("Uyari: Mevcut yazar ID'si sistemde kayitli degil.\n");
    }

    if (new_yazar_id != -1 && !find_yazar_by_id(ktp, new_yazar_id))
    {
        printf("Uyari: Yeni yazar ID'si sistemde kayitli degil. Islem iptal edildi.\n");
        return;
    }

    // Remove all existing author relationships for this book
    for (size_t i = 0; i < ktp->ky_list.count;)
    {
        if (strcmp(ktp->ky_list.dizi[i].isbn, isbn) == 0)
        {
            // Remove this entry by shifting remaining elements
            free(ktp->ky_list.dizi[i].isbn);
            for (size_t j = i; j < ktp->ky_list.count - 1; j++)
            {
                ktp->ky_list.dizi[j] = ktp->ky_list.dizi[j + 1];
            }
            ktp->ky_list.count--;
        }
        else
        {
            i++;
        }
    }

    // Add new author relationship or mark as deleted (-1)
    if (new_yazar_id != -1)
    {
        int added = add_kitap_yazar(ktp, isbn, new_yazar_id);
        if (added == 1)
        {
            printf("Yeni yazar (%d) basariyla eklendi.\n", new_yazar_id);
        }
        else if (added == 0)
        {
            printf("Yeni yazar (%d) zaten kitapla eslestirilmis.\n", new_yazar_id);
        }
        else
        {
            printf("Yeni yazar eklenirken hata olustu.\n");
        }
    }
    else
    {
        // Add -1 to indicate deleted author
        int added = add_kitap_yazar(ktp, isbn, -1);
        if (added == 1)
        {
            printf("Yazar tamamen kaldirildi ve -1 olarak isaretlendi.\n");
        }
        else
        {
            printf("Yazar kaldirma islemi basarisiz oldu.\n");
        }
    }

    // Save changes to file
    if (save_kitap_yazar("KitapYazar.csv", ktp) != 0)
    {
        printf("Hata: Kitap-Yazar iliskileri kaydedilemedi.\n");
    }
}

void kitapOduncBirak(kutuphane *ktp)
{
    int choice;
    char isbn_etiket[50];
    int ogrenci_no;
    char date_str[11];
    int c;

    printf("\nKitap Islemleri:\n");
    printf("1. Kitap odunc Al\n");
    printf("2. Kitap Teslim Et\n");
    printf("Seciminiz: ");
    if (scanf("%d", &choice) != 1 || (choice != 1 && choice != 2))
    {
        printf("Gecersiz secim.\n");
        while (getchar() != '\n')
            ;
        return;
    }
    while (getchar() != '\n')
        ;

    printf("Ogrenci Numarasi: ");
    if (scanf("%d", &ogrenci_no) != 1)
    {
        printf("Gecersiz ogrenci numarasi.\n");
        while (getchar() != '\n')
            ;
        return;
    }
    while (getchar() != '\n')
        ;

    printf("Islem Tarihi (dd.mm.yyyy): ");
    if (fgets(date_str, sizeof(date_str), stdin) == NULL)
    {
        printf("Tarih okuma hatasi.\n");
        return;
    }
    kes(date_str);

    int day, month, year;
    if (sscanf(date_str, "%d.%d.%d", &day, &month, &year) != 3 ||
        day < 1 || day > 31 || month < 1 || month > 12 || year < 1900)
    {
        printf("Gecersiz tarih formati.\n");
        return;
    }

    if (choice == 1)
    {
        while ((c = getchar()) != '\n' && c != EOF)
            ;

        printf("\nMevcut Kitaplar:\n");
        printf("ISBN\t\tKitap Adi\n");
        printf("--------------------------------\n");
        for (kitap *k = ktp->kitap_head; k; k = k->next)
        {
            int available = 0;
            for (kitap_ornek *o = k->ornekler; o; o = o->next)
            {
                if (o->rafta_mi)
                {
                    available = 1;
                    break;
                }
            }
            if (available)
            {
                printf("%s\t%s\n", k->isbn, k->isim);
            }
        }
        printf("--------------------------------\n");

        printf("\nOdunc alinacak kitabin ISBN'i: ");
        if (fgets(isbn_etiket, sizeof(isbn_etiket), stdin) == NULL)
        {
            printf("ISBN okuma hatasi.\n");
            return;
        }
        kes(isbn_etiket);
        bosluklariKes(isbn_etiket);

        if (strlen(isbn_etiket) == 0)
        {
            printf("Hata: ISBN bos olamaz.\n");
            return;
        }
        if (!isbn_kontrol(isbn_etiket))
        {
            printf("Gecersiz ISBN format. 13 haneli sayi olmalidir.\n");
            return;
        }

        int result = borrow_book(ktp, isbn_etiket, ogrenci_no, date_str);
        if (result == 0)
        {
            printf("Kitap basariyla odunc alindi.\n");
            save_kitap_ornekleri("kitap_ornekleri.csv", ktp);
            save_islemler("islemler.csv", ktp);
        }
        else
        {
            switch (result)
            {
            case -1:
                printf("Hata: Ogrenci bulunamadi.\n");
                break;
            case -2:
                printf("Hata: Ogrencinin puani 0'in altinda, kitap odunc alamaz.\n");
                break;
            case -3:
                printf("Hata: Kitap bulunamadi.\n");
                break;
            case -4:
                printf("Hata: Kitabin rafta uygun kopyasi bulunmamaktadir.\n");
                break;
            default:
                printf("Hata: %d\n", result);
            }
        }
    }
    else
    {
        while ((c = getchar()) != '\n' && c != EOF)
            ;

        printf("Teslim edilecek kitabin etiket numarasi: ");
        memset(isbn_etiket, 0, sizeof(isbn_etiket)); // Clear buffer
        if (fgets(isbn_etiket, sizeof(isbn_etiket), stdin) == NULL)
        {
            printf("Etiket No okuma hatasi.\n");
            return;
        }
        kes(isbn_etiket);
        bosluklariKes(isbn_etiket);

        if (strlen(isbn_etiket) == 0)
        {
            printf("Hata: Etiket numarasi bos olamaz.\n");
            return;
        }

        int result = return_book(ktp, isbn_etiket, ogrenci_no, date_str);
        if (result == 0)
        {
            printf("Kitap başarıyla teslim edildi.\n");
            save_kitap_ornekleri("kitap_ornekleri.csv", ktp);
            save_islemler("islemler.csv", ktp);
            save_ogrenciler("ogrenciler.csv", ktp); // For potential point updates
        }
        else
        {
            printf("Hata: %d\n", result);
        }
    }
}

typedef int (*LoadFunction)(const char *, kutuphane *);
typedef int (*SaveFunction)(const char *, kutuphane *);

typedef struct
{
    const char *filename;
    LoadFunction load;
    SaveFunction save;
} FileOperation;

static const FileOperation file_ops[] = {
    {"Yazarlar.csv", load_yazarlar, save_yazarlar},
    {"Ogrenciler.csv", load_ogrenciler, save_ogrenciler},
    {"Kitaplar.csv", load_kitaplar, save_kitaplar},
    {"KitapOrnekleri.csv", load_kitap_ornekleri, save_kitap_ornekleri},
    {"KitapYazar.csv", load_kitap_yazar, save_kitap_yazar},
    {"KitapOdunc.csv", load_islemler, save_islemler},
    {NULL, NULL, NULL} // Sentinel
};

int load_all_data(kutuphane *ktp)
{
    for (const FileOperation *op = file_ops; op->filename != NULL; op++)
    {
        if (op->load(op->filename, ktp) != 0)
        {
            printf("Hata: %s dosyasi yuklenemedi.\n", op->filename);
            return -1;
        }
    }
    return 0;
}

int save_all_data(kutuphane *ktp)
{
    for (const FileOperation *op = file_ops; op->filename != NULL; op++)
    {
        if (op->save(op->filename, ktp) != 0)
        {
            printf("Hata: %s dosyasina kaydedilemedi.\n", op->filename);
            return -1;
        }
    }
    return 0;
}

int save_specific_data(kutuphane *ktp, const char *filename)
{
    for (const FileOperation *op = file_ops; op->filename != NULL; op++)
    {
        if (strcmp(op->filename, filename) == 0)
        {
            return op->save(filename, ktp);
        }
    }
    return -1;
}

int main()
{
    setlocale(LC_ALL, "tr_TR.UTF-8");

    kutuphane *ktp = malloc(sizeof(kutuphane));
    ktp->ogrenci_head = NULL;
    ktp->yazar_head = NULL;
    ktp->kitap_head = NULL;
    ktp->islem_head = NULL;
    ktp->ky_list.dizi = NULL;
    ktp->ky_list.count = 0;
    ktp->ky_list.capacity = 0;

    if (load_all_data(ktp) != 0)
    {
        printf("Veri yuklenirken hata olustu.\n");
        free_kutuphane(ktp);
        return 1;
    }

    int choice;
    char input_buffer[256];
    char current_date[11];
    get_current_date_str(current_date);
    do
    {
        printf("\n--- KUTUPHANE OTOMASYONU MENUSU ---\n");
        printf("OGRENCI ISLEMLERI:\n");
        printf("1. Ogrenci Ekle, Sil, Guncelle\n");
        printf("2. Ogrenci Bilgisi Goruntuleme\n");
        printf("3. Kitap Teslim Etmemis Ogrencileri Listele\n");
        printf("4. Cezali Ogrencileri Listele\n");
        printf("5. Tum Ogrencileri Listele\n");
        printf("6. Kitap Odunc Al/Teslim Et\n");

        printf("\nKITAP ISLEMLERI:\n");
        printf("7. Kitap Ekle, Sil, Guncelle\n");
        printf("8. Kitap Bilgisi Goruntuleme\n");
        printf("9. Raftaki Kitaplari Listele\n");
        printf("10. Zamaninda Teslim Edilmeyen Kitaplari Listele\n");
        printf("11. Kitap-Yazar Eslestir\n");
        printf("12. Kitabin Yazarini Guncelle\n");

        printf("\nYAZAR ISLEMLERI:\n");
        printf("13. Yazar Ekle, Sil, Guncelle\n");
        printf("14. Yazar Bilgisi Goruntuleme\n");

        printf("0. Cikis\n");
        printf("Seciminiz: ");
        scanf("%d", &choice);
        while (getchar() != '\n')
            ;

        switch (choice)
        {
        case 1:
        {
            int sub_choice;
            printf("\nogrenci islemleri:\n");
            printf("1. ogrenci Ekle\n");
            printf("2. ogrenci Sil\n");
            printf("3. ogrenci Guncelle\n");
            printf("Seciminiz: ");
            scanf("%d", &sub_choice);
            while (getchar() != '\n')
                ;

            if (sub_choice == 1)
            {
                ogrenciEkle(ktp);
            }
            else if (sub_choice == 2)
            {
                int student_id;
                printf("Silinecek ogrenci numarasi: ");
                scanf("%d", &student_id);
                while (getchar() != '\n')
                    ;
                if (ogrenciSil(ktp, student_id) == 0)
                {
                    save_ogrenciler("ogrenciler.csv", ktp);
                    save_kitap_ornekleri("kitap_ornekleri.csv", ktp);
                    printf("ogrenci basariyla silindi.\n");
                }
                else
                {
                    printf("ogrenci bulunamadi veya silinirken hata olustu.\n");
                }
            }
            else if (sub_choice == 3)
            {
                ogrenciGuncelle(ktp);
            }
            else
            {
                printf("Gecersiz alt secim.\n");
            }
            break;
        }
        case 2:
        {
            int student_id;
            printf("Goruntulenecek ogrenci ID'si: ");
            scanf("%d", &student_id);
            while (getchar() != '\n')
                ;
            printOgrenci(ktp, student_id);
            break;
        }
        case 3:
        {
            list_students_with_overdue_books(ktp, current_date);
            break;
        }
        case 4:
        {
            list_students_with_penalty(ktp);
            break;
        }
        case 5:
        {
            ogrencileriListele(ktp);
            break;
        }
        case 6:
        {
            kitapOduncBirak(ktp);
            break;
        }
        case 7:
        {
            int sub_choice;
            printf("\nKitap Islemleri:\n");
            printf("1. Kitap Ekle\n");
            printf("2. Kitap Sil\n");
            printf("3. Kitap Guncelle\n");
            printf("Seciminiz: ");
            scanf("%d", &sub_choice);
            while (getchar() != '\n')
                ;

            if (sub_choice == 1)
            {
                kitapEkle(ktp);
            }
            else if (sub_choice == 2)
            {
                char isbn[15];
                printf("Silinecek kitabin ISBN'i: ");
                fgets(isbn, sizeof(isbn), stdin);
                kes(isbn);
                if (kitapSil(ktp, isbn) == 0)
                {
                    save_kitaplar("kitaplar.csv", ktp);
                    save_kitap_ornekleri("kitap_ornekleri.csv", ktp);
                    save_kitap_yazar("kitap_yazar.csv", ktp);
                    printf("Kitap basariyla silindi.\n");
                }
                else
                {
                    printf("Kitap bulunamadi veya silinirken hata olustu.\n");
                }
            }
            else if (sub_choice == 3)
            {
                kitapGuncelle(ktp);
            }
            else
            {
                printf("Gecersiz alt secim.\n");
            }
            break;
        }
        case 8:
        {
            char book_name[256];
            printf("Goruntulenecek kitabin adi: ");
            fgets(book_name, sizeof(book_name), stdin);
            kes(book_name);
            printKitap(ktp, book_name);
            break;
        }
        case 9:
        {
            raftakiKitaplariListele(ktp);
            break;
        }
        case 10:
        {
            gecikmisKitapListele(ktp, current_date);
            break;
        }
        case 11:
        {
            match_book_author(ktp);
            break;
        }
        case 12:
        {
            kitapyazariniGuncelle(ktp);
            break;
        }
        case 13:
        {
            int sub_choice;
            printf("\nYazar Islemleri:\n");
            printf("1. Yazar Ekle\n");
            printf("2. Yazar Sil\n");
            printf("3. Yazar Guncelle\n");
            printf("Seciminiz: ");
            scanf("%d", &sub_choice);
            while (getchar() != '\n')
                ;

            if (sub_choice == 1)
            {
                yazarEKle(ktp);
            }
            else if (sub_choice == 2)
            {
                int author_id;
                printf("Silinecek yazar ID'si: ");
                scanf("%d", &author_id);
                while (getchar() != '\n')
                    ;
                if (delete_yazar(ktp, author_id) == 0)
                {
                    save_yazarlar("yazarlar.csv", ktp);
                    save_kitap_yazar("kitap_yazar.csv", ktp);
                    printf("Yazar basariyla silindi.\n");
                }
                else
                {
                    printf("Yazar bulunamadi veya silinirken hata olustu.\n");
                }
            }
            else if (sub_choice == 3)
            {
                update_author(ktp);
            }
            else
            {
                printf("Gecersiz alt secim.\n");
            }
            break;
        }
        case 14:
        {
            char authorID[100];
            printf("Goruntulenecek yazarin ID: ");
            fgets(authorID, sizeof(authorID), stdin);
            kes(authorID);
            printYazar(ktp, authorID);
            break;
        }
        case 0:
        {
            printf("Programdan cikiliyor...\n");
            break;
        }
        default:
        {
            printf("Gecersiz secim, lutfen tekrar deneyin.\n");
            break;
        }
        }
    } while (choice != 0);

    // Save all data before exiting
    save_all_data(ktp);
    printf("saving all data ...");
    free_kutuphane(ktp);
    return 0;
}
