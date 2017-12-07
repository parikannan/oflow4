hi clear Normal
set bg&

" Remove all existing highlighting and set the defaults.
hi clear

if exists("syntax_on")
    syntax reset
endif
let colors_name = "pari"

"hi Normal       ctermfg=7 
hi NonText      ctermfg=white
hi comment      ctermfg=2 
hi constant     ctermfg=3 
"hi identifier   ctermfg=white cterm=bold
hi identifier   ctermfg=7
hi statement    ctermfg=6
hi preproc      ctermfg=5 cterm=bold 
hi type         ctermfg=6
hi special      ctermfg=3
hi ErrorMsg     ctermfg=0 ctermbg=3 
hi WarningMsg   ctermfg=0 ctermbg=4 
hi ModeMsg      ctermfg=0 ctermbg=5 
hi MoreMsg      ctermfg=0 ctermbg=2 
hi Error        ctermfg=0 ctermbg=3 
hi Todo         ctermfg=0 ctermbg=3 
"hi Cursor       ctermfg=white  
hi Search       term=underline cterm=underline ctermfg=7 ctermbg=4 
hi IncSearch    term=reverse cterm=NONE ctermfg=0 ctermbg=2 
hi LineNr       ctermfg=5 
hi title        ctermfg=5 
hi StatusLineNC ctermfg=0 ctermbg=7 
hi StatusLine   ctermfg=0 ctermbg=7 cterm=bold 
hi label        ctermfg=5 
hi operator     ctermfg=magenta
hi clear Visual
hi Visual       term=reverse ctermbg=blue 
"hi DiffChange   ctermfg=5 
"hi DiffText     ctermfg=5 
"hi DiffAdd      ctermfg=5 
"hi DiffDelete   ctermfg=5 
"hi Folded       ctermfg=5 
"hi FoldColumn   ctermfg=5 
"hi cIf0         ctermfg=5

"ground, blink = bright background) colours.   The  canonical  names  are  as  follows:
"             0=black,  1=red, 2=green, 3=yellow, 4=blue, 5=magenta, 6=cyan, 7=white, but the actual
"             colour names used are listed in the COLORS AND GRAPHICS section.
