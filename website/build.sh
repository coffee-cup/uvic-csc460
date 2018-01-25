#!/bin/bash

SRCDIR=src
OUTDIR=build

# Projects
PROJECTS=("project1" "project2")

OPTS="-f markdown -t html5 --template template.html --mathjax"

# Create build directory
mkdir -p $OUTDIR

# Create a tmp directory to use
mkdir -p ./tmp

# Run pandoc on all md files at top of src dir
for f in $SRCDIR/*.md ;
do
    pandoc $OPTS "${f}" -s -o "$OUTDIR/$(basename ${f} .md).html"  ;
done

# Projects
for f in "${PROJECTS[@]}"
do
    POUT=$OUTDIR/${f}
    TMP=./tmp/${f}
    mkdir -p $POUT

    # Concat all md files into tmp one
    for file in $SRCDIR/${f}/*.md
    do
        (cat "$file"; echo '') >> $TMP
    done

    # Run pandoc on tmp file to generate html
    pandoc $OPTS $TMP -o "$POUT/index.html"
done

# Copy css to build dir
cp pandoc.css $OUTDIR

# Remove tmp
rm -rf ./tmp
