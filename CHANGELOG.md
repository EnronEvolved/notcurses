This document attempts to list user-visible changes and any major internal
rearrangements of Notcurse.

* 1.2.5 (not yet released)
** gradient, polyfill, ncplane_format() and ncplane_stain() all now return the
   number of cells written on success. Failure still sees -1 returned.
** `ncvisual_render()` now returns the number of cells emitted on success, as
    opposed to 0. Failure still sees -1 returned.
** `ncvisual_render()` now interprets length parameters of -1 to mean "to the
   end along this axis", and no longer interprets 0 to mean this. 0 now means
   "a length of 0", resulting in a zero-area rendering.
** `notcurses_at_yx()` no longer accepts a `cell*` as its last parameter.
   Instead, it accepts a `uint32_t*` and a `uint64_t*`, and writes the
   attribute and channels to these parameters. This was done because the
   `gcluster` field of the `cell*` was always set to 0, which was surprising
   and a source of blunders. The EGC is returned via the `char*` return
   value. https://github.com/dankamongmen/notcurses/issues/410

* 1.2.4 2020-03-24
** Add ncmultiselector
** Add `ncdirect_cursor_enable()` and `ncdirect_cursor_disable()`.