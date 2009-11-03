#!/bin/csh

if (-d testft) then
    rm -rf testft
endif

mkdir testft

time seqret -feat tembl:\* embl::testft/orig.embl -auto

time seqret -feat embl::testft/orig.embl embl::testft/copy.embl -auto

diff testft/orig.embl testft/copy.embl >! testft/copy.diff

time seqret -feat embl::testft/orig.embl -srev embl::testft/rev.embl -debug -auto
time seqret -feat embl::testft/rev.embl -srev embl::testft/restore.embl -auto


diff testft/orig.embl testft/restore.embl >! testft/restore.diff
