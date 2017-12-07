" this file for older vims < 600
"hi clear
"if exists("syntax_on")
"    syntax reset
"endif
"let g:colors_name = "pari"

hi Normal       ctermfg=7 guifg=white guibg=darkblue
hi NonText      guifg=#cc33cc
hi comment      ctermfg=2 guifg=gray      gui=bold
hi constant     ctermfg=3 guifg=cyan
hi identifier   ctermfg=12 guifg=gray
hi statement    ctermfg=6 guifg=yellow    gui=none
hi preproc      ctermfg=5 guifg=green
hi type         ctermfg=6 guifg=orange
hi special      ctermfg=3 guifg=#cc33cc
hi ErrorMsg     ctermfg=0 ctermbg=3 guifg=orange    guibg=darkblue
hi WarningMsg   ctermfg=0 ctermbg=4 guifg=cyan      guibg=darkblue  gui=bold
hi ModeMsg      ctermfg=0 ctermbg=5 guifg=yellow    gui=NONE
hi MoreMsg      ctermfg=0 ctermbg=2 guifg=yellow    gui=NONE
hi Error        ctermfg=0 ctermbg=3 guifg=Red       guibg=darkblue
hi Todo         ctermfg=0 ctermbg=3 guifg=Black     guibg=orange
hi Cursor       ctermfg=7  guifg=Black guibg=white
hi Search       term=underline cterm=underline ctermfg=7 ctermbg=4 guifg=Black     guibg=orange
hi IncSearch    term=reverse cterm=NONE ctermfg=0 ctermbg=2 guifg=Black     guibg=steelblue gui=NONE
hi LineNr       ctermfg=5 guifg=pink
hi title        ctermfg=5 guifg=white gui=bold
hi StatusLineNC ctermfg=0 ctermbg=7 gui=NONE    guifg=white guibg=blue
hi StatusLine   ctermfg=0 ctermbg=7 cterm=bold gui=bold    guifg=cyan  guibg=blue
hi label        ctermfg=5 guifg=gold2
hi operator     guifg=orange    gui=bold
hi clear Visual
hi Visual       term=reverse cterm=reverse gui=reverse
hi DiffChange   ctermfg=5 guibg=darkgreen guifg=black
hi DiffText     ctermfg=5 guibg=olivedrab guifg=black
hi DiffAdd      ctermfg=5 guibg=slateblue guifg=black
hi DiffDelete   ctermfg=5 guibg=coral     guifg=black
hi Folded       ctermfg=5 guibg=orange guifg=black
hi FoldColumn   ctermfg=5 guibg=gray30 guifg=black
hi cIf0         ctermfg=5 guifg=gray

"ground, blink = bright background) colours.   The  canonical  names  are  as  follows:
"             0=black,  1=red, 2=green, 3=yellow, 4=blue, 5=magenta, 6=cyan, 7=white, but the actual
"             colour names used are listed in the COLORS AND GRAPHICS section.
