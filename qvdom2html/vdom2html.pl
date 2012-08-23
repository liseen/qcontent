package VDOM::Html;

use strict;
use warnings;
use encoding 'utf8';

sub node2html ($$);
sub escape_txt ($);
sub gen_html_attrs ($$);
sub txtnode2html ($$);

sub go {
    my ($win, $ref_html) = @_;
    $$ref_html .= "<html>\n" .
        "<head>\n" .
        qq{<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />\n}.
        "<title>" . escape_txt($win->document->title) . "</title>\n".
        "</head>\n";
    node2html($win->document->body, $ref_html);
    $$ref_html .= "</html>\n";
}

sub node2html ($$) {
    my ($node, $ref_html) = @_;
    if ($node->nodeType == $VDOM::Node::TEXT_NODE) {
         txtnode2html($node, $ref_html);
    } else {
        $$ref_html .= '<' . $node->tagName;
        gen_html_attrs($node, $ref_html);
        $$ref_html .= ">\n";
        for my $child ($node->childNodes) {
            node2html($child, $ref_html);
        }
        $$ref_html .= "\n</" . $node->tagName . ">\n";
    }
}

sub escape_txt ($) {
    my $text = shift;
    for ($text) {
        s/&/&amp;/g;
        s/</&lt;/g;
        s/>/&gt;/g;
        s/"/&quot;/g;
        #s/\n/ /g;
    }
    $text;
}

sub txtnode2html ($$) {
    my ($node, $ref_html) = @_;
    my $parent = $node->parentNode;
    my ($left, $top);
    if (defined $parent->x) {
        $left = $node->x - $parent->x;
        $top = $node->y - $parent->y;
    } else {
        $left = $node->x;
        $top = $node->y;
    }
    my $style = 'position: absolute;' .
        "left: $left;" .
        "top: $top;" .
        "width: " . $node->w . ';' .
        "height: " . $node->h . ';';
    $$ref_html .= qq{<span style="} . escape_txt($style) . qq{">}.
        escape_txt($node->nodeValue) . qq{</span>\n};
}

sub gen_html_attrs ($$) {
    my ($elem, $ref_html) = @_;
    my @attrs;
    if (defined $elem->id) {
        push @attrs, qq{id="} . escape_txt($elem->id) . qq{"};
    }
    if (defined $elem->className) {
        push @attrs, qq{class="} . escape_txt($elem->className) . qq{"};
    }
    if (defined $elem->href) {
        push @attrs, qq{href="} . escape_txt($elem->href) . qq{"};
    }
    if (defined $elem->name) {
        push @attrs, qq{name="} . escape_txt($elem->name) . qq{"};
    }
    if (defined $elem->title) {
        push @attrs, qq{title="} . escape_txt($elem->title) . qq{"};
    }
    if (defined $elem->src) {
        push @attrs, qq{src="} . escape_txt($elem->src) . qq{"};
    }
    if (defined $elem->alt) {
        push @attrs, qq{alt="} . escape_txt($elem->alt) . qq{"};
    }
    my $parent = $elem->parentNode;
    my ($left, $top);
    if (defined $parent->x) {
        $left = $elem->x - $parent->x;
        $top = $elem->y - $parent->y;
    } else {
        $left = $elem->x;
        $top = $elem->y;
    }
    my $style = 'position: absolute;' .
        "left: $left;" .
        "top: $top;" .
        "width: " . $elem->w . ';' .
        "height: " . $elem->h . ';'.
        "color: " . $elem->color . ";" .
        "background-color: " . $elem->backgroundColor . ";" .
        "font-size: " . $elem->fontSize . ";" .
        "font-weight: " . $elem->fontWeight . ";".
        "font-family: " . $elem->fontFamily . ";" .
        "font-style: " . $elem->fontStyle . ";";
    push @attrs, qq{style="} . escape_txt($style) . qq{"};
    if (@attrs) {
        $$ref_html .= " " . join(' ', @attrs);
    }
}

1;

