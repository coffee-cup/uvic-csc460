#!/bin/bash

SRCDIR=src
OUTDIR=build

# Projects
PROJECTS=("project1")

OPTS="-f markdown -t html5 --template template.html --mathjax"
OPTS_PDF="-f markdown --template template.tex --listings --toc"

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
    # Check for an index file in the project
    PINDEX=$SRCDIR/${f}/index.txt
    if [ -f $PINDEX ]; then

        POUT=$OUTDIR/${f}
        TMP=./tmp/${f}
        mkdir -p $POUT

        # Using index order, concat all md files into tmp one
        while read file; do
            if [ -f $SRCDIR/${f}/$file ]; then
                (cat "$SRCDIR/${f}/$file"; echo '') >> $TMP
            fi
        done < $PINDEX

        # Run pandoc on tmp file to generate pdfs
        if [ "$PDF" = "1" ]
        then
            pandoc $OPTS_PDF $TMP -o "$POUT/${f}.pdf"
        fi

        # Run pandoc on tmp file to generate html
        pandoc $OPTS --toc $TMP -o "$POUT/index.html"
    fi

done

# Copy css to build dir
cp pandoc.css $OUTDIR

# Remove tmp
rm -rf ./tmp
