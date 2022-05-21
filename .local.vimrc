let &path.="header,src,"
nnoremap <A-m> :AsyncRun build.bat<cr>
nnoremap <A-p> :AsyncStop<cr>
nnoremap <A-r> :AsyncRun -mode=async run.bat<cr>
nnoremap <F7> :cn<cr>
set guifont=JetBrains\ Mono:h11
nnoremap gd :YcmCompleter GoTo<CR>

autocmd BufNewFile,BufRead *.sh_it set ft=sh_it
autocmd BufNewFile,BufRead *.sh_it set commentstring=//\ %s

set noexpandtab
set errorformat=%f(%l):\ %m,%f(%l)\ :\ %m

