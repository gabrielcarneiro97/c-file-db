#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

int find_id(char *str, char **strs, int strs_len) {
  for (int i = 0; i < strs_len; i += 1) {
    if (strcmp(str, strs[i]) == 0) {
      return i;
    }
  }

  return -1;
}

char* concat(const char *s1, const char *s2) {
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    char *result = malloc(len1 + len2 + 1); // +1 for the null-terminator

    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1);
    return result;
}

void _log(char *message, char *file, int line) {
  printf("%s::%d: %s\n", file, line, message);
}

char **str_split(char* a_str, const char a_delim, size_t *len) {

  char **result = 0;
  size_t count = 0;
  char* tmp = a_str;
  char* last_comma = 0;
  char delim[2];
  delim[0] = a_delim;
  delim[1] = 0;

  /* Count how many elements will be extracted. */
  while (*tmp) {
    if (a_delim == *tmp) {
      count += 1;
      last_comma = tmp;
    }
    tmp += 1;
  }

  /* Add space for trailing token. */
  count += last_comma < (a_str + strlen(a_str) - 1);

  if (len != NULL) *len = count;

  /* Add space for terminating null string so caller
      knows where the list of returned strings ends. */
  count += 1;

  result = malloc(sizeof(char*) * count);

  if (result) {
    size_t idx  = 0;
    char* token = strtok(a_str, delim);

    while (token) {
      assert(idx < count);
      *(result + idx++) = strdup(token);
      token = strtok(0, delim);
    }
    assert(idx == count - 1);
    *(result + idx) = 0;
  }

  return result;
}

char *str_clone(char *str) {
  size_t len = strlen(str);

  char *new_str = (char *) malloc(sizeof(char *) * len);

  strcpy(new_str, str);

  return new_str;
}

char **file_to_lines(char *filename, int *lines_len) {
  FILE *file = fopen(filename, "r");

  if (file == NULL) {
    _log("file is a NULL pointer", __FILE__, __LINE__);
  }

  char *line = NULL;
  size_t len = 0;

  char **lines = (char **) malloc(sizeof(char **));
  int counter = 0;

  while (getline(&line, &len, file) != -1) {
    lines[counter] = (char *) malloc(sizeof(char *) * len);
    strcpy(lines[counter], line);

    counter += 1;

    lines = realloc(lines, sizeof(char **) * (counter + 1));
  }

  lines[counter] = NULL;

  if (lines_len != NULL) *lines_len = counter;

  fclose(file);

  return lines;
}

typedef struct file_desc {
  char *filename;

  char **cols;
  char **types;
  int c_counter;

  char **rows;
  int r_counter;

} file_desc;

file_desc new_file_desc(char *filename) {
  file_desc desc;
  desc.filename = filename;

  desc.c_counter = 0;
  desc.r_counter = 0;

  desc.cols = NULL;
  desc.types = NULL;
  desc.rows = NULL;

  return desc;
}

void _add_row(file_desc *desc, char *row) {
  if (desc->rows == NULL) {
    desc->rows = (char **) malloc(sizeof(char **));
  } else {
    desc->rows = (char **) realloc(desc->rows, sizeof(char **) * desc->r_counter + 1);
  }

  desc->rows[desc->r_counter] = str_clone(row);

  desc->r_counter += 1;
}

void _add_col(file_desc *desc, char *col, char *type) {
  if (type == NULL) return;

  if (desc->cols == NULL) {
    desc->cols = (char **) malloc(sizeof(char **));
    desc->types = (char **) malloc(sizeof(char **));
  } else {
    desc->cols = (char **) realloc(desc->cols, sizeof(char **) * desc->c_counter + 1);
    desc->types = (char **) realloc(desc->types, sizeof(char **) * desc->c_counter + 1);
  }

  desc->cols[desc->c_counter] = str_clone(col);
  desc->types[desc->c_counter] = str_clone(type);

  desc->c_counter += 1;
}

void add_col(file_desc *desc, char *col_type) {
  char **extract = str_split(col_type, ':', NULL);
  _add_col(desc, extract[0], extract[1]);
}

void print_cols(file_desc desc) {
  for (int i = 0; i < desc.c_counter; i += 1) {
    printf("%s:%s\n", desc.cols[i], desc.types[i]);
  }
}

void print_rows(file_desc desc) {
  for (int i = 0; i < desc.r_counter; i += 1) {
    printf("%d -> %s", i, desc.rows[i]);
  }
}

file_desc describe_file(char *filename) {
  int lines_counter = 0;
  char **lines = file_to_lines(filename, &lines_counter);

  char *header = lines[0];

  size_t c_counter;
  char **col_types = str_split(header, ';', &c_counter);

  file_desc desc = new_file_desc(filename);

  for(int i = 0; i < c_counter; i += 1) {
    add_col(&desc, col_types[i]);
  }

  for (int i = 1; i < lines_counter; i += 1) {
    _add_row(&desc, lines[i]);
  }

  return desc;
}

char **get_row(file_desc desc, char *col, char *val) {
  int col_id = find_id(col, desc.cols, desc.c_counter);

  for (int i = 0; i < desc.r_counter; i += 1) {
    char *str_row = str_clone(desc.rows[i]);
    char **row = str_split(str_row, ';', NULL);

    if (strcmp(row[col_id], val) == 0) return row;
  }

  return NULL;
}

typedef struct pessoa {
  int id;
  char *nome;
} pessoa;

void check_filename(file_desc desc, char *expected, char *file, int line) {
    if (strcmp(desc.filename, expected) != 0) {
    _log("file_desc fornecido inválido", file, line);
    exit(0);
  }
}

pessoa get_pessoa(file_desc desc, int id) {
  check_filename(desc, "pessoa.csv", __FILE__, __LINE__);

  char str_id[100];
  sprintf(str_id, "%d", id);
  char **row = get_row(desc, "id", str_id);

  pessoa p;

  if (row != NULL) {
    p.id = atoi(row[0]);
    p.nome = row[1];
  }

  return p;
}

int main() {
  file_desc desc = describe_file("pessoa.csv");

  pessoa p = get_pessoa(desc, 1);

  // printf("%d\n", desc.r_counter);

  print_rows(desc);

  // printf("%d, %s\n", p.id, p.nome);

  return 0;
}
