#include "internal.h"

static void
free_menu_section(struct ncmenu_section* ms){
  for(int i = 0 ; i < ms->itemcount ; ++i){
    free(ms->items[i].desc);
  }
  free(ms->items);
  free(ms->name);
}

static void
free_menu_sections(ncmenu* ncm){
  for(int i = 0 ; i < ncm->sectioncount ; ++i){
    free_menu_section(&ncm->sections[i]);
  }
  free(ncm->sections);
}

static int
dup_menu_section(struct ncmenu_section* dst, const struct ncmenu_section* src){
  dst->items = NULL;
  if(src->itemcount){
    dst->items = malloc(sizeof(*dst->items) * src->itemcount);
    if(dst->items == NULL){
      return -1;
    }
    for(int i = 0 ; i < src->itemcount ; ++i){
      if((dst->items[i].desc = strdup(src->items[i].desc)) == NULL){
        while(--i){
          free(&dst->items[i].desc);
        }
      }
    }
  }
  return 0;
}

// Duplicates all menu sections in opts, adding their length to '*totalwidth'.
static int
dup_menu_items(ncmenu* ncm, const ncmenu_options* opts, int* totalwidth, int* totalheight){
  ncm->sections = NULL;
  if((ncm->sectioncount = opts->sectioncount) == 0){
    ++*totalwidth; // one character margin on right
    return 0;
  }
  ncm->sections = malloc(sizeof(*ncm->sections) * opts->sectioncount);
  if(ncm->sections == NULL){
    return -1;
  }
  int maxheight = 0;
  for(int i = 0 ; i < opts->sectioncount ; ++i){
    int cols = mbswidth(opts->sections[i].name);
    if(cols < 0 || (ncm->sections[i].name = strdup(opts->sections[i].name)) == NULL){
      while(--i){
        free_menu_section(&ncm->sections[i]);
      }
    }
    *totalwidth += cols + 2;
    if(dup_menu_section(&opts->sections[i], &ncm->sections[i])){
      free(ncm->sections[i].name);
      while(--i){
        free_menu_section(&ncm->sections[i]);
      }
      return -1;
    }
    if(ncm->sections[i].itemcount > maxheight){
      maxheight = ncm->sections[i].itemcount;
    }
  }
  *totalheight += maxheight + 2; // two rows of border
  return 0;
}

static int
write_header(ncmenu* ncm){
  ncm->ncp->channels = ncm->headerchannels;
  int dimy, dimx;
  ncplane_dim_yx(ncm->ncp, &dimy, &dimx);
  int xoff = 2; // 2-column margin on left
  int ypos = ncm->bottom ? dimy - 1 : 0;
  if(ncplane_cursor_move_yx(ncm->ncp, ypos, 0)){
    return -1;
  }
  cell c = CELL_INITIALIZER(' ', 0, ncm->headerchannels);
  if(ncplane_putc(ncm->ncp, &c) < 0){
    return -1;
  }
  if(ncplane_putc(ncm->ncp, &c) < 0){
    return -1;
  }
  for(int i = 0 ; i < ncm->sectioncount ; ++i){
    if(ncplane_putstr(ncm->ncp, ncm->sections[i].name) < 0){
      return -1;
    }
    if(ncplane_putc(ncm->ncp, &c) < 0){
      return -1;
    }
    if(ncplane_putc(ncm->ncp, &c) < 0){
      return -1;
    }
    xoff += mbswidth(ncm->sections[i].name) + 2;
  }
  while(xoff++ < dimx){
    if(ncplane_putc(ncm->ncp, &c) < 0){
      return -1;
    }
  }
  return 0;
}

ncmenu* ncmenu_create(notcurses* nc, const ncmenu_options* opts){
  if(opts->sectioncount < 0){
    return NULL;
  }else if(opts->sectioncount == 0 && opts->sections){
    return NULL;
  }else if(opts->sectioncount && !opts->sections){
    return NULL;
  }
  int totalheight = 1;
  int totalwidth = 1; // start with one character margin on the left
  // FIXME calculate maximum dimensions
  ncmenu* ret = malloc(sizeof(*ret));
  ret->sectioncount = opts->sectioncount;
  ret->sections = NULL;
  int dimy, dimx;
  ncplane_dim_yx(notcurses_stdplane(nc), &dimy, &dimx);
  if(ret){
    ret->bottom = opts->bottom;
    // FIXME maximum width could be more than section headers, due to items!
    if(dup_menu_items(ret, opts, &totalwidth, &totalheight) == 0){
      ret->headerwidth = totalwidth;
      if(totalwidth < dimx){
        totalwidth = dimx;
      }
      int ypos = opts->bottom ? dimy - totalheight : 0;
      ret->ncp = ncplane_new(nc, totalheight, totalwidth, ypos, 0, NULL);
      if(ret->ncp){
        ret->unrolledsection = -1;
        ret->headerchannels = opts->headerchannels;
        ret->sectionchannels = opts->sectionchannels;
        if(write_header(ret) == 0){
          return ret;
        }
        ncplane_destroy(ret->ncp);
      }
      free_menu_sections(ret);
    }
    free(ret);
  }
  return NULL;
}

int ncmenu_unroll(ncmenu* n, int sectionidx){
  if(sectionidx < 0 || sectionidx >= n->sectioncount){
    return -1;
  }
  if(ncmenu_rollup(n)){ // roll up any unrolled section
    return -1;
  }
  n->unrolledsection = sectionidx;
  // FIXME
  return 0;
}

int ncmenu_rollup(ncmenu* n){
  if(n->unrolledsection < 0){
    return 0;
  }
  // FIXME
  return 0;
}

int ncmenu_destroy(ncmenu* n){
  int ret = 0;
  if(n){
    free_menu_sections(n);
    ncplane_destroy(n->ncp);
    free(n);
  }
  return ret;
}