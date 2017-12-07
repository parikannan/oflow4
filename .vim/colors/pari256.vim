hi clear Normal
set bg&

" Remove all existing highlighting and set the defaults.
hi clear

if exists("syntax_on")
    syntax reset
endif
let colors_name = "pari"

hi Normal       ctermfg=248
hi NonText      ctermfg=253
hi comment      ctermfg=041 
hi constant     ctermfg=252
"hi identifier   ctermfg=white cterm=bold
hi identifier   ctermfg=7
"hi statement    ctermfg=6
hi statement    ctermfg=66
hi preproc      ctermfg=5 
hi type         ctermfg=30
hi special      ctermfg=3
hi ErrorMsg     ctermfg=0 ctermbg=202
hi WarningMsg   ctermfg=0 ctermbg=190
hi ModeMsg      ctermfg=0 ctermbg=82
hi MoreMsg      ctermfg=0 ctermbg=2 
hi Error        ctermbg=124 
hi Todo         ctermfg=0 ctermbg=3 
"hi Cursor       ctermfg=white  
hi Search       term=underline cterm=underline ctermfg=7 ctermbg=027
hi IncSearch    cterm=NONE ctermfg=7 ctermbg=027
hi LineNr       ctermfg=241 ctermbg=235 
hi title        ctermfg=5 
hi StatusLineNC ctermfg=0 ctermbg=245
hi StatusLine   ctermfg=252 ctermbg=59
hi label        ctermfg=5 
hi operator     ctermfg=magenta
hi clear Visual
hi Visual       term=reverse ctermbg=blue 
hi PreProc      ctermfg=153
hi DiffChange   ctermfg=None ctermbg=238
hi DiffText   ctermfg=None ctermbg=241
hi DiffAdd ctermfg=None ctermbg=241
"hi DiffDelete   ctermfg=5 
"hi Folded       ctermfg=5 
"hi FoldColumn   ctermfg=5 
"hi cIf0         ctermfg=5

"ground, blink = bright background) colours.   The  canonical  names  are  as  follows:
"             0=black,  1=red, 2=green, 3=yellow, 4=blue, 5=magenta, 6=cyan, 7=white, but the actual
"             colour names used are listed in the COLORS AND GRAPHICS section.
