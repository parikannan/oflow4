"----------------------------------------------------------------------------------
" File      : .vimrc
" Descr     : pari's customized vim startup file
" Author    : parik
" History   : Created -> Ancient times (pre 98)
"           : Modded 2004-08-02 12:20:47 (better perl support)
"				: Modded 2004-08-05 17:31:46 (enable multi-word searches =* ) 
"				: Fixed 2004-09-24 17:18:48  perl comments being auto-unindented. inoremap
"				: Changed all TABs to 2 spaces 2010-01-21 08:46:11  
"----------------------------------------------------------------------------------
set showbreak=+++\      " Precede continued screen lines
set shiftwidth=2        " Indent by three columns at a time. Chg to 2.
set tabstop=2        " Indent by three columns at a time. Chg to 2
set incsearch           " Incremental searching
set sidescroll=8        " Horizontal scrolling 8 columns at a time
set showmatch           " Show matching delimiters
set showmode            " Show current input mode in status line
set ruler               " Enable ruler on status line
set scrolloff=2         " Keep 2 lines above and below cursor
set laststatus=2        " Always show a status line
set smartcase           " Ignore ignorecase for uppercase letters in patterns
set splitbelow          " Split windows below current window
set ttyscroll=5         " Scroll at most 5 lines at a time
set winheight=4         " At least 4 lines for current window
set cinoptions=:0,p0,t0,(1s                 " C language indent options
"set lazyredraw
set writebackup
set nobackup
set backspace=indent,eol,start	"winbloz style backspace effect in INSERT

" change xterm title to filename.
"set title titlestring=%t
" do not change title on quit
"set titleold=" "

"syntax enable
syntax on

"set dir=~/tmp		" location of .swp files. prevents dir pollution
set dir=$VIMSWPDIR " location of .swp files. prevents dir pollution

" 2011-05-28 09:31:21 retain undo history on buffer changes. 
set hidden 

" 2011-08-31 11:10:22  
let &titleold=$MYTERMTITLE
set title

if version >= 600
  "colorscheme kolor
  colorscheme pari_dull
  "colorscheme murphy
  "colorscheme slate
else
	so ~/.vim/colors/pari_dull_500.vim
	set t_kb=
	fixdel
endif

"if version >= 700
"  map <F10> :tabn<CR>
"  map <F9> :tabp<CR>
"  map <F3> :tabnew<CR>
"endif

" common tcl file snippets
ab abt1 set_param -name place.hardVerbose -value 469538set_param -name route.flowDbg -value 1set_param -name route.timingDbg -value 1#set_param -name route.dbgNetId  -value 1#set_param -name route.dbgPrintExpand  -value 4#set_param -name route.netlistBuildDbg  -value 5set_param write_ncd.noDrc trueset_param -name route.writeBudgetFile -value true
ab abt2 write_checkpoint -force -file postplace.dcpwrite_ncd -force -file postplace.ncdwrite_pcf -force -file postplace.pcf
ab abt3 write_checkpoint -force -file postroute.dcpwrite_ncd -force -file postroute.ncdwrite_pcf -force -file postroute.pcf

nmap =s /Starting Routing Taskma
nmap =w /WNS
nmap =y /Post Router Timing
nmap =t /Total Node Congestion               =
nmap =0 /Total Node Congestion               = 0
nmap =c /Checksum
nmap =x /Total Expand
nmap =r /route_design
nmap =p /HPWL

"colorscheme pari
"colorscheme blue

"set highlight=8:SpecialKey,@:NonText,d:Directory,e:ErrorMsg,i:IncSearch,l:Search,m:MoreMsg,M:ModeMsg,n:LineNr,r:Question,s:StatusLine+++ ,t:Title,v:Visual,w:WarningMsg

nmap <F6> <C-W><C-W>
imap <F6> <C-O><C-W><C-W>
nmap <S-F6> <C-W>W
imap <S-F6> <C-O><C-W>W
nmap <F7> :bprevious<CR>
imap <F7> <C-O>:bprevious<CR>
nmap <F8> :bnext<CR>
imap <F8> <C-O>:bnext<CR>
nmap <S-F9> gqapj
vmap <S-F9> gq
nnoremap <F4> :set hlsearch!<CR>:set hlsearch?<CR>
map <C-P> 1<C-Y>
map <C-N> 1<C-E>
nnoremap gp :set paste!<CR>:set paste?<CR>
nmap <F5> "=strftime("%Y-%m-%d %H:%M:%S")<CR>Pa 

"insert date
"nmap `D a <ESC>mdi<C-R>=strftime("%Y-%m-%d %H:%M:%S")<CR><Esc>
noremap `D a <ESC>"=strftime("%Y-%m-%d %H:%M:%S")<CR>P<Esc>
imap `D <ESC>`Da 
imap `- ------------------------------------------------------------------------------

"set verbose=9

"script to enable multi-word searches and highlights
"use =* to select the word and repeat for additional words. dunno how it works
noremap =*     :call MultiWordSearch()<C-M>
noremap =<C-N> :call MultiWordSearch()<C-M>
function! MultiWordSearch()
	let @/ = @/ . '\|\<' . expand("<cword>") . '\>'
	normal nh
	call histadd("search",@/)
endfunction

" add markers to current line
sign define fixme text=!! linehl=Todo texthl=Error
sign define mysign text=>> texthl=Search
noremap <F5> :exe ":sign place ".line(".")." line=".line(".")." name=fixme file=".expand("%:p")<CR><Esc>
noremap <S-F5> :exe ":sign unplace ".line(".")<CR><Esc>

if has("autocmd")
 augroup filetype
 	au!
   au! BufRead,BufNewFile *.t    set filetype=make
 augroup END
endif

set smarttab expandtab softtabstop=2 autoindent smartindent

if has("autocmd")
 augroup cprog
    au!
"	 au! BufRead,BufNewFile *.c *.h *.cc set filetype=cpp
"	 autocmd FileType c,cpp,cc,hh,h set filetype=cpp
	 autocmd FileType c,h set filetype=cpp
     "autocmd FileType c,cpp,cc,hh,h	set noignorecase smarttab expandtab nolinebreak cinwords=if,else,while,do,for,switch nosmartindent noautoindent cindent comments=sr:/*,mb:*,el:*/,b:// formatoptions=croq2 textwidth=80 softtabstop=3 
     autocmd FileType c,cpp,cxx,cc,hh,h	set noignorecase smarttab expandtab nolinebreak cinwords=if,else,while,do,for,switch nosmartindent noautoindent cindent comments=sr:/*,mb:*,el:*/,b:// formatoptions=croq2 textwidth=80 softtabstop=2 
     autocmd FileType c,cpp,cc,hh,h	imap `n <ESC>a#include 
     autocmd FileType c,cpp,cc,hh,h	imap `d <ESC>a#define 
     autocmd FileType c,cpp,cc,hh,h     vmap `0 <Esc>'<O#if 0<Esc>'>o#endif<Esc><C-O><C-O>
     autocmd FileType c,cpp,cc,hh,h     imap `- //----------------------------------------------------------------------------<CR>//----------------------------------------------------------------------------<Esc>:set textwidth=72<CR>O//
     autocmd FileType c,cpp,cc,hh,h     nmap `- O`-
     autocmd FileType c,cpp,cc,hh,h     vmap `- y`-<C-R>"<CR><Tab>
     autocmd FileType c,cpp,cc,hh,h     imap `\  //////////////////////////////////////////////////////////////////////////////<CR>// 
     autocmd FileType c,cpp,cc,hh,h     nmap `\ O`\
     autocmd FileType c,cpp,cc,hh,h     nmap `* _2O/*<CR><CR><BS>/<Up><Space>
     autocmd FileType c,cpp,cc,hh,h     imap `* <ESC>`*
     autocmd FileType c,cpp,cc,hh,h     vmap `/ :s/^\\(    \\\|\\t\\)*/&\\/\\/ /<CR>
     autocmd FileType c,cpp,cc,hh,h     vmap `\\ :s/^\\(\\(    \\\|\\t\\)*\\)\\(\\/\\/ \\\| \\*  \\)/\\1<CR>:'\<,'\>g/^\\(    \\\|\\t\\)*\\(\\/\\*\\\| \\*\\/\\)\\( \\\|\\t\\)*+++ $/d<CR>
 augroup END
endif

if has("autocmd")
 augroup perlprog
	au!
	autocmd FileType pl set filetype=perl
	autocmd FileType perl set noignorecase smarttab expandtab nolinebreak textwidth=80 softtabstop=2 autoindent nocindent smartindent cinwords=if,else,elsif,for comments=b:# formatoptions=croq2

   autocmd FileType perl imap `n <ESC>a#!/usr/bin/perl -w

	autocmd FileType perl imap `- #------------------------------------------------------------------------<CR>#------------------------------------------------------------------------<Esc>:set textwidth=72<CR>O# 
	autocmd FileType perl nmap `- O`-
	autocmd FileType perl vmap `- y`-<C-R>"<CR><Tab>

   autocmd FileType perl imap `\  #/////////////////////////////////////////////////////////////////////<CR># 
   autocmd FileType perl nmap `\ O`\
   autocmd FileType perl vmap `\ :s/^/# <CR>
	inoremap # X#

 augroup END
endif

fun! ShowFuncName()
  let lnum = line(".")
  let col = col(".")
  "echohl ModeMsg
  echo getline(search("^[^ \t#/]\\{2}.*[^:]\s*$", 'bW'))
  "echohl None
  call search("\\%" . lnum . "l" . "\\%" . col . "c")
endfun
map f :call ShowFuncName() <CR>

set tabpagemax=100

