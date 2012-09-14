## Process this file with automake to produce Makefile.in

SUBDIRS = acd data index

AM_CFLAGS  = $(WARN_CFLAGS) $(DEVWARN_CFLAGS)
if ISSHARED
if ISAIXIA64
AM_CFLAGS += -Wl,-bdynamic
AM_CFLAGS += -Wl,-brtl
AM_CFLAGS += -L../ajax/pcre/.libs
AM_CFLAGS += -L../ajax/expat/.libs
AM_CFLAGS += -L../ajax/zlib/.libs
AM_CFLAGS += -L../ajax/core/.libs
AM_CFLAGS += -L../ajax/graphics/.libs
AM_CFLAGS += -L../ajax/ensembl/.libs
AM_CFLAGS += -L../ajax/ajaxdb/.libs
AM_CFLAGS += -L../ajax/acd/.libs
AM_CFLAGS += -lacd
AM_CFLAGS += -lajaxdb
AM_CFLAGS += -lensembl
AM_CFLAGS += -lajaxg
AM_CFLAGS += -lajax
AM_CFLAGS += -leexpat
AM_CFLAGS += -lepcre
AM_CFLAGS += -lezlib
endif
endif

AM_CPPFLAGS  = -I$(top_srcdir)/ajax/acd
AM_CPPFLAGS += -I$(top_srcdir)/ajax/ajaxdb
AM_CPPFLAGS += -I$(top_srcdir)/ajax/ensembl
AM_CPPFLAGS += -I$(top_srcdir)/ajax/graphics
AM_CPPFLAGS += -I$(top_srcdir)/ajax/pcre
if !ESYSTEMLIBS
AM_CPPFLAGS += -I$(top_srcdir)/ajax/expat
AM_CPPFLAGS += -I$(top_srcdir)/ajax/zlib
endif
AM_CPPFLAGS += -I$(top_srcdir)/nucleus
AM_CPPFLAGS += -I$(top_srcdir)/plplot
AM_CPPFLAGS += $(AXIS2C_CPPFLAGS)
AM_CPPFLAGS += $(JAVA_CPPFLAGS)

LDADD  = ../nucleus/libnucleus.la
LDADD += ../ajax/acd/libacd.la
LDADD += ../ajax/ajaxdb/libajaxdb.la
LDADD += ../ajax/ensembl/libensembl.la
LDADD += ../ajax/graphics/libajaxg.la
LDADD += ../ajax/core/libajax.la
if !ESYSTEMLIBS
LDADD += ../ajax/zlib/libezlib.la
LDADD += ../ajax/expat/libeexpat.la
endif
LDADD += ../ajax/pcre/libepcre.la
LDADD += ../plplot/libeplplot.la
LDADD += $(XLIB)

#if PURIFY
#LINK = purify $(LIBTOOL) --mode=link $(CC) $(CFLAGS) $(LDFLAGS) -o $@
#endif

# Applications, which are not built.
#
# dbget
# dbgeturl
# dbshowdat
# dbshowquery
# dbtellquery
# docxmlvalid
# docxmlwell
# entrygo
# entrytax
# goseq
# isdbdata
# isdbname
# seqgo
# seqtax
# taxseq
# template ###
# texttotext
# texttoxml
# textvalid
# xmltotext
# xmltoxml
# xmltransform

check_PROGRAMS = \
acdrelations \
acdxsd \
ajtest \
complex \
corbatest \
dastest \
dbfetchtest \
domtesta \
domtestb \
domtestc \
domtestd \
edamclean \
ensembltest \
entrails \
entrailsbook \
entrailshtml \
entrailswiki \
histogramtest \
intconv \
martattributes \
martdatasets \
martfilters \
martquery \
martregistry \
martseqs \
newcoils \
origsplitter \
origunion \
patmattest \
prima \
primers \
seqinfo \
seqretall \
seqretallfeat \
seqretset \
seqretsingle \
seqrettype \
sqltest \
testplot \
treetypedisplay

bin_PROGRAMS = \
aaindexextract \
abiview \
acdc \
acdgalaxy \
acdlog \
acdpretty \
acdtable \
acdtrace \
acdvalid \
aligncopy \
aligncopypair \
antigenic \
assemblyget \
backtranambig \
backtranseq \
banana \
biosed \
btwisted \
cachedas \
cachedbfetch \
cacheebeyesearch \
cacheensembl \
cai \
chaos \
charge \
checktrans \
chips \
cirdna \
codcmp \
codcopy \
coderet \
compseq \
cons \
consambig \
cpgplot \
cpgreport \
cusp \
cutgextract \
cutseq \
dan \
dbiblast \
dbifasta \
dbiflat \
dbigcg \
dbtell \
dbxcompress \
dbxedam \
dbxfasta \
dbxflat \
dbxgcg \
dbxobo \
dbxreport \
dbxresource \
dbxstat \
dbxtax \
dbxuncompress \
degapseq \
density \
descseq \
diffseq \
distmat \
dotmatcher \
dotpath \
dottup \
dreg \
drfinddata \
drfindformat \
drfindid \
drfindresource \
drget \
drtext \
edamdef \
edamhasinput \
edamhasoutput \
edamisformat \
edamisid \
edamname \
edialign \
einverted \
embossdata \
embossupdate \
embossversion \
emma \
emowse \
entret \
epestfind \
eprimer3 \
eprimer32 \
equicktandem \
est2genome \
etandem \
extractalign \
extractfeat \
extractseq \
featcopy \
featmerge \
featreport \
feattext \
findkm \
freak \
fuzznuc \
fuzzpro \
fuzztran \
garnier \
geecee \
getorf \
godef \
goname \
helixturnhelix \
hmoment \
iep \
infoalign \
infoassembly \
infobase \
inforesidue \
infoseq \
isochore \
jaspextract \
jaspscan \
jembossctl \
lindna \
listor \
makenucseq \
makeprotseq \
marscan \
maskambignuc \
maskambigprot \
maskfeat \
maskseq \
matcher \
megamerger \
merger \
msbar \
mwcontam \
mwfilter \
needle \
needleall \
newcpgreport \
newcpgseek \
newseq \
nohtml \
noreturn \
nospace \
notab \
notseq \
nthseq \
nthseqset \
octanol \
oddcomp \
ontocount \
ontoget \
ontogetcommon \
ontogetdown \
ontogetobsolete \
ontogetroot \
ontogetsibs \
ontogetup \
ontoisobsolete \
ontotext \
palindrome \
pasteseq \
patmatdb \
patmatmotifs \
pepcoil \
pepdigest \
pepinfo \
pepnet \
pepstats \
pepwheel \
pepwindow \
pepwindowall \
plotcon \
plotorf \
polydot \
preg \
prettyplot \
prettyseq \
primersearch \
printsextract \
profit \
prophecy \
prophet \
prosextract \
pscan \
psiphi \
rebaseextract \
recoder \
redata \
refseqget \
remap \
restover \
restrict \
revseq \
seealso \
seqcount \
seqmatchall \
seqret \
seqretsetall \
seqretsplit \
seqxref \
seqxrefget \
servertell \
showalign \
showdb \
showfeat \
showorf \
showpep \
showseq \
showserver \
shuffleseq \
sigcleave \
silent \
sirna \
sixpack \
sizeseq \
skipredundant \
skipseq \
splitsource \
splitter \
stretcher \
stssearch \
supermatcher \
syco \
taxget \
taxgetdown \
taxgetrank \
taxgetspecies \
taxgetup \
tcode \
textget \
textsearch \
tfextract \
tfm \
tfscan \
tmap \
tranalign \
transeq \
trimest \
trimseq \
trimspace \
twofeat \
union \
urlget \
variationget \
vectorstrip \
water \
whichdb \
wobble \
wordcount \
wordfinder \
wordmatch \
wossdata \
wossinput \
wossname \
wossoperation \
wossoutput \
wossparam \
wosstopic \
yank

dist_pkgdata_DATA = \
emboss.default.template \
emboss.standard \
server.biomart \
server.dasregistry \
server.dbfetch server.dkfz \
server.ebeye \
server.entrez \
server.ensembl \
server.ensemblgenomes \
server.mrs \
server.mrs3 \
server.mrs4 \
server.srs \
server.wsdbfetch

# EMBOSS bin application SOURCES

aaindexextract_SOURCES = aaindexextract.c
abiview_SOURCES = abiview.c
acdc_SOURCES = acdc.c
acdgalaxy_SOURCES = acdgalaxy.c
acdpretty_SOURCES = acdpretty.c
acdtable_SOURCES = acdtable.c
acdtrace_SOURCES = acdtrace.c
acdvalid_SOURCES = acdvalid.c
aligncopy_SOURCES = aligncopy.c
aligncopypair_SOURCES = aligncopypair.c
antigenic_SOURCES = antigenic.c
assemblyget_SOURCES = assemblyget.c
backtranambig_SOURCES = backtranambig.c
backtranseq_SOURCES = backtranseq.c
banana_SOURCES = banana.c
biosed_SOURCES = biosed.c
btwisted_SOURCES = btwisted.c
cachedas_SOURCES = cachedas.c
cachedbfetch_SOURCES = cachedbfetch.c
cacheebeyesearch_SOURCES = cacheebeyesearch.c
cacheensembl_SOURCES = cacheensembl.c
cai_SOURCES = cai.c
chaos_SOURCES = chaos.c
charge_SOURCES = charge.c
checktrans_SOURCES = checktrans.c
chips_SOURCES = chips.c
cirdna_SOURCES = cirdna.c
codcmp_SOURCES = codcmp.c
codcopy_SOURCES = codcopy.c
coderet_SOURCES = coderet.c
compseq_SOURCES = compseq.c
cons_SOURCES = cons.c
consambig_SOURCES = consambig.c
cpgplot_SOURCES = cpgplot.c
cpgreport_SOURCES = cpgreport.c
cusp_SOURCES = cusp.c
cutgextract_SOURCES = cutgextract.c
cutseq_SOURCES = cutseq.c
dan_SOURCES = dan.c
dbiblast_SOURCES = dbiblast.c
dbifasta_SOURCES = dbifasta.c
dbiflat_SOURCES = dbiflat.c
dbigcg_SOURCES = dbigcg.c
dbtell_SOURCES = dbtell.c
dbxcompress_SOURCES = dbxcompress.c
dbxedam_SOURCES = dbxedam.c
dbxfasta_SOURCES = dbxfasta.c
dbxflat_SOURCES = dbxflat.c
dbxgcg_SOURCES = dbxgcg.c
dbxobo_SOURCES = dbxobo.c
dbxreport_SOURCES = dbxreport.c
dbxresource_SOURCES = dbxresource.c
dbxstat_SOURCES = dbxstat.c
dbxtax_SOURCES = dbxtax.c
dbxuncompress_SOURCES = dbxuncompress.c
degapseq_SOURCES = degapseq.c
density_SOURCES = density.c
descseq_SOURCES = descseq.c
diffseq_SOURCES = diffseq.c
distmat_SOURCES = distmat.c
dotmatcher_SOURCES = dotmatcher.c
dotpath_SOURCES = dotpath.c
dottup_SOURCES = dottup.c
dreg_SOURCES = dreg.c
drfinddata_SOURCES = drfinddata.c
drfindformat_SOURCES = drfindformat.c
drfindid_SOURCES = drfindid.c
drfindresource_SOURCES = drfindresource.c
drget_SOURCES = drget.c
drtext_SOURCES = drtext.c
edamdef_SOURCES = edamdef.c
edamhasinput_SOURCES = edamhasinput.c
edamhasoutput_SOURCES = edamhasoutput.c
edamisformat_SOURCES = edamisformat.c
edamisid_SOURCES = edamisid.c
edamname_SOURCES = edamname.c
edialign_SOURCES = edialign.c
einverted_SOURCES = einverted.c
embossdata_SOURCES = embossdata.c
embossversion_SOURCES = embossversion.c
emma_SOURCES = emma.c
emowse_SOURCES = emowse.c
entret_SOURCES = entret.c
epestfind_SOURCES = epestfind.c
eprimer3_SOURCES = eprimer3.c
eprimer32_SOURCES = eprimer32.c
equicktandem_SOURCES = equicktandem.c
est2genome_SOURCES = est2genome.c
etandem_SOURCES = etandem.c
extractalign_SOURCES = extractalign.c
extractfeat_SOURCES = extractfeat.c
extractseq_SOURCES = extractseq.c
featcopy_SOURCES = featcopy.c
featmerge_SOURCES = featmerge.c
featreport_SOURCES = featreport.c
feattext_SOURCES = feattext.c
findkm_SOURCES = findkm.c
freak_SOURCES = freak.c
fuzznuc_SOURCES = fuzznuc.c
fuzzpro_SOURCES = fuzzpro.c
fuzztran_SOURCES = fuzztran.c
garnier_SOURCES = garnier.c
geecee_SOURCES = geecee.c
getorf_SOURCES = getorf.c
godef_SOURCES = godef.c
goname_SOURCES = goname.c
helixturnhelix_SOURCES = helixturnhelix.c
hmoment_SOURCES = hmoment.c
iep_SOURCES = iep.c
infoalign_SOURCES = infoalign.c
infoassembly_SOURCES = infoassembly.c
infobase_SOURCES = infobase.c
inforesidue_SOURCES = inforesidue.c
infoseq_SOURCES = infoseq.c
isochore_SOURCES = isochore.c
jaspextract_SOURCES = jaspextract.c
jaspscan_SOURCES = jaspscan.c
jembossctl_SOURCES = jembossctl.c
lindna_SOURCES = lindna.c
listor_SOURCES = listor.c
makenucseq_SORCES = makenucseq.c
makeprotseq_SOURCES = makeprotseq.c
marscan_SOURCES = marscan.c
maskambignuc_SOURCES = maskambignuc.c
maskambigprot_SOURCES = maskambigprot.c
maskfeat_SOURCES = maskfeat.c
maskseq_SOURCES = maskseq.c
matcher_SOURCES = matcher.c
megamerger_SOURCES = megamerger.c
merger_SOURCES = merger.c
msbar_SOURCES = msbar.c
mwcontam_SOURCES = mwcontam.c
mwfilter_SOURCES = mwfilter.c
needle_SOURCES = needle.c
needleall_SOURCES = needleall.c
newcpgreport_SOURCES = newcpgreport.c
newcpgseek_SOURCES = newcpgseek.c
newseq_SOURCES = newseq.c
nohtml_SOURCES = nohtml.c
noreturn_SOURCES = noreturn.c
nospace_SOURCES = nospace.c
notab_SOURCES = notab.c
notseq_SOURCES = notseq.c
nthseq_SOURCES = nthseq.c
nthseqset_SOURCES = nthseqset.c
octanol_SOURCES = octanol.c
oddcomp_SOURCES = oddcomp.c
ontocount_SOURCES = ontocount.c
ontoget_SOURCES = ontoget.c
ontogetcommon_SOURCES = ontogetcommon.c
ontogetdown_SOURCES = ontogetdown.c
ontogetobsolete_SOURCES = ontogetobsolete.c
ontogetroot_SOURCES = ontogetroot.c
ontogetsibs_SOURCES = ontogetsibs.c
ontogetup_SOURCES = ontogetup.c
ontoisobsolete_SOURCES = ontoisobsolete.c
ontotext_SOURCES = ontotext.c
palindrome_SOURCES = palindrome.c
pasteseq_SOURCES = pasteseq.c
patmatdb_SOURCES = patmatdb.c
patmatmotifs_SOURCES = patmatmotifs.c
pepcoil_SOURCES = pepcoil.c
pepdigest_SOURCES = pepdigest.c
pepinfo_SOURCES = pepinfo.c
pepnet_SOURCES = pepnet.c
pepstats_SOURCES = pepstats.c
pepwheel_SOURCES = pepwheel.c
pepwindow_SOURCES = pepwindow.c
pepwindowall_SOURCES = pepwindowall.c
plotcon_SOURCES = plotcon.c
plotorf_SOURCES = plotorf.c
polydot_SOURCES = polydot.c
preg_SOURCES = preg.c
prettyplot_SOURCES = prettyplot.c
prettyseq_SOURCES = prettyseq.c
primersearch_SOURCES = primersearch.c
printsextract_SOURCES = printsextract.c
profit_SOURCES = profit.c
prophecy_SOURCES = prophecy.c
prophet_SOURCES = prophet.c
prosextract_SOURCES = prosextract.c
pscan_SOURCES = pscan.c
psiphi_SOURCES = psiphi.c
rebaseextract_SOURCES = rebaseextract.c
recoder_SOURCES = recoder.c
redata_SOURCES = redata.c
refseqget_SOURCES = refseqget.c
remap_SOURCES = remap.c
restover_SOURCES = restover.c
restrict_SOURCES = restrict.c
revseq_SOURCES = revseq.c
seealso_SOURCES = seealso.c
seqcount_SOURCES = seqcount.c
seqmatchall_SOURCES = seqmatchall.c
seqret_SOURCES = seqret.c
seqretsetall_SOURCES = seqretsetall.c
seqretsplit_SOURCES = seqretsplit.c
seqxref_SOURCES = seqxref.c
seqxrefget_SOURCES = seqxrefget.c
servertell_SOURCES = servertell.c
showalign_SOURCES = showalign.c
showdb_SOURCES = showdb.c
showfeat_SOURCES = showfeat.c
showorf_SOURCES = showorf.c
showpep_SOURCES = showpep.c
showseq_SOURCES = showseq.c
showserver_SOURCES = showserver.c
shuffleseq_SOURCES = shuffleseq.c
sigcleave_SOURCES = sigcleave.c
silent_SOURCES = silent.c
sirna_SOURCES = sirna.c
sixpack_SOURCES = sixpack.c
sizeseq_SOURCES = sizeseq.c
skipredundant_SOURCES = skipredundant.c
skipseq_SOURCES = skipseq.c
splitsource_SOURCES = splitsource.c
splitter_SOURCES = splitter.c
stretcher_SOURCES = stretcher.c
stssearch_SOURCES = stssearch.c
supermatcher_SOURCES = supermatcher.c
syco_SOURCES = syco.c
taxget_SOURCES = taxget.c
taxgetdown_SOURCES = taxgetdown.c
taxgetrank_SOURCES = taxgetrank.c
taxgetspecies_SOURCES = taxgetspecies.c
taxgetup_SOURCES = taxgetup.c
tcode_SOURCES = tcode.c
textget_SOURCE = textget.c
textsearch_SOURCES = textsearch.c
tfextract_SOURCES = tfextract.c
tfm_SOURCES = tfm.c
tfscan_SOURCES = tfscan.c
tmap_SOURCES = tmap.c
tranalign_SOURCES = tranalign.c
transeq_SOURCES = transeq.c
trimest_SOURCES = trimest.c
trimseq_SOURCES = trimseq.c
trimspace_SOURCES = trimspace.c
twofeat_SOURCES = twofeat.c
union_SOURCES = union.c
urlget_SOURCES = urlget.c
variationget_SOURCES = variationget.c
vectorstrip_SOURCES = vectorstrip.c
water_SOURCES = water.c
whichdb_SOURCES = whichdb.c
wobble_SOURCES = wobble.c
wordcount_SOURCES = wordcount.c
wordfinder_SOURCES = wordfinder.c
wordmatch_SOURCES = wordmatch.c
wossdata_SOURCES = wossdata.c
wossinput_SOURCES = wossinput.c
wossname_SOURCES = wossname.c
wossoperation_SOURCES = wossoperation.c
wossoutput_SOURCES = wossoutput.c
wossparam_SOURCES = wossparam.c
wosstopic_SOURCES = wosstopic.c
yank_SOURCES = yank.c

# EMBOSS check application SOURCES

acdlog_SOURCES = acdlog.c
acdrelations_SOURCES = acdrelations.c
acdxsd_SOURCES = acdxsd.c
ajtest_SOURCES = ajtest.c
complex_SOURCES = complex.c
corbatest_SOURCES = corbatest.c
dastest_SOURCES = dastest.c
dbfetchtest_SOURCES = dbfetchtest.c
domtesta_SOURCES = domtesta.c
domtestb_SOURCES = domtestb.c
domtestc_SOURCES = domtestc.c
domtestd_SOURCES = domtestd.c
edamclean_SOURCES = edamclean.c
ensembltest_SOURCES = ensembltest.c
entrails_SOURCES = entrails.c
entrailsbook_SOURCES = entrailsbook.c
entrailshtml_SOURCES = entrailshtml.c
entrailswiki_SOURCES = entrailswiki.c
histogramtest_SOURCES = histogramtest.c
intconv_SOURCES = intconv.c
martattributes_SOURCES = martattributes.c
martdatasets_SOURCES = martdatasets.c
martfilters_SOURCES = martfilters.c
martquery_SOURCES = martquery.c
martregistry_SOURCES = martregistry.c
martseqs_SOURCES = martseqs.c
newcoils_SOURCES = newcoils.c
origsplitter_SOURCES = origsplitter.c
origunion_SOURCES = origunion.c
patmattest_SOURCES = patmattest.c
prima_SOURCES = prima.c
primers_SOURCES = primers.c
seqinfo_SOURCES = seqinfo.c
seqretall_SOURCES = seqretall.c
seqretallfeat_SOURCES = seqretallfeat.c
seqretset_SOURCES = seqretset.c
seqretsingle_SOURCES = seqretsingle.c
seqrettype_SOURCES = seqrettype.c
sqltest_SOURCES = sqltest.c
testplot_SOURCES = testplot.c
treetypedisplay_SOURCES = treetypedisplay.c
