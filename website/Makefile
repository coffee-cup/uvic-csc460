SRCDIR="src"
OUTDIR="build"

MARKDOWN=$(shell find $(SRCDIR) -iname "*.md")
HTML_1=$(MARKDOWN:.md=.html)
HTML := $(addprefix $(OUTDIR)/,$(notdir $(HTML_1)))

.PHONY = all clean
all: $(HTML) copy

$(HTML): $(MARKDOWN)
	pandoc -f markdown -t html5 \
		--template template.html \
		-c pandoc.css \
		-A footer.html \
		--mathjax \
		$< -o $@

copy: pandoc.css
	cp pandoc.css $(OUTDIR)

clean:
	rm $(HTML)
