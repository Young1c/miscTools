#!/bin/bash

# Follow the artile of Firmin Martin
# https://firminmartin.com/en/posts/2021/05/git_autocomplete_trailer_and_recipients_in_zsh/

# Collect all the Name <address> for a certain project
# __build_contact()
# {
# 	git shortlog -sne --all | cut -f2
# }

__git_select_address()
{
	local __data="No data"
	local __contact=~/bin/git_contacts.dat

	[[ -f $__contact ]] && __data=$(cat $__contact)

	echo $__data | fzf -m --reverse |
        while read item; do
                echo -n "${item} "
        done
}

__git_show_adderss()
{
  LBUFFER="${LBUFFER}$(__git_select_address)"
  zle redisplay
  typeset -f zle-line-init >/dev/null && zle zle-line-init
}

# Bind to a shortcut, ctrl-alt-a
zle     -N	__git_show_adderss
bindkey '^[^A'	__git_show_adderss

