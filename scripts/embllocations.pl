#!/usr/bin/perl -w

$id = "unknown";
$hasrange = 0;			# nn..nn
$hassingle = 0;			# nn
$hasbetween = 0;		# nn^nn
$hasbetweencirc = 0;		# nn^1
$hasset = 0;			# (nn.nn)
$hasjoin = 0;			# join(nn..nn,nn..nn)
$hasorder = 0;			# order(
$hasgroup = 0;			# group(
$hasoneof = 0;			# one_of(
$hasbefore = 0;			# <nn
$hasafter = 0;			# >nn
$hascomplement = 0;		# complement(nn..nn)
$hasjoincomplement = 0;		# join(complement(nn..nn),
$hascomplementjoin = 0;		# complement(join(nn..nn),

$has = 1;

@types = ("range", "single", "between", "betweencirc",
	  "set","join","order","group",
	  "one_of","before","after",
	  "complement","joincomplement","complementjoin",
    );
%ids = ();
%locations = ();
while (<>) {
    if(/^ID +(\S+)/) {
	$id = $1;
	$id =~ s/;$//;
	$textstr = $_;
    }
    elsif(/^LOCUS +(\S+)/) {
	$id = $1;
	$id =~ s/;$//;
	$textstr = $_;
    }
    else {
	$textstr .= $_;
    }

    if(/^[F ][T ]   (\S+) +(.*)/) {
	$location = $2;
	if(!$hasrange){
	    if($location =~ /^(\d+)\.\.(\d+)$/) {
		$hasrange++;
		$ids{"range"} = $id;
		$locations{"range"} = $location;
	    }
	}
	if(!$hassingle){
	    if($location =~ /^\d+$/) {
		$hassingle++;
		$ids{"single"} = $id;
		$locations{"single"} = $location;
	    }
	}
	if(!$hasbetween){
	    if($location =~ /^\d+[\^]\d+$/) {
		$hasbetween++;
		$ids{"between"} = $id;
		$locations{"between"} = $location;
	    }
	}
	if(!$hasbetweencirc){
	    if($location =~ /^\d+[\^]1$/) {
		$hasbetweencirc++;
		$ids{"betweencirc"} = $id;
		$locations{"betweencirc"} = $location;
	    }
	}
	if(!$hasset){
	    if($location =~ /\d+[\.]\d+/) {
		$hasset++;
		$ids{"set"} = $id;
		$locations{"set"} = $location;
	    }
	}
	if(!$hasjoin){
	    if($location =~ /^join[\(]\d+\.\.\d+,[0-9.\(\),]+$/) {
		$hasjoin++;
		$ids{"join"} = $id;
		$locations{"join"} = $location;
	    }
	}
	if(!$hasorder){
	    if($location =~ /^order[\(]\d+\.\.\d+,[0-9.\(\),]+$/) {
		$hasorder++;
		$ids{"order"} = $id;
		$locations{"order"} = $location;
	    }
	}
	if(!$hasgroup){
	    if($location =~ /^group[\(]\d+\.\.\d+,[0-9.\(\),]+$/) {
		$hasgroup++;
		$ids{"group"} = $id;
		$locations{"group"} = $location;
	    }
	}
	if(!$hasoneof){
	    if($location =~ /^one_of[\(]\d+\.\.\d+,[0-9.\(\),]+$/) {
		$hasoneof++;
		$ids{"one_of"} = $id;
		$locations{"one_of"} = $location;
	    }
	}
	if(!$hasbefore){
	    if($location =~ /[<]\d+\.\./) {
		$hasbefore++;
		$ids{"before"} = $id;
		$locations{"before"} = $location;
	    }
	}
	if(!$hasafter){
	    if($location =~ /\.\.[>]\d+/) {
		$hasafter++;
		$ids{"after"} = $id;
		$locations{"after"} = $location;
	    }
	}
	if(!$hascomplement){
	    if($location =~ /^complement[\(]\d+\.\.\d+[\)]$/) {
		$hascomplement++;
		$ids{"complement"} = $id;
		$locations{"complement"} = $location;
	    }
	}
	if(!$hasjoincomplement){
	    if($location =~ /^join[\(].*complement[\(]\d+\.\.\d+[\)]/) {
		$hasjoincomplement++;
		$ids{"joincomplement"} = $id;
		$locations{"joincomplement"} = $location;
	    }
	}
	if(!$hascomplementjoin){
	    if($location =~ /^complement[\(]join[\(]\d+\.\.\d+/) {
		$hascomplementjoin++;
		$ids{"complementjoin"} = $id;
		$locations{"complementjoin"} = $location;
	    }
	}
	if(!$has){
	    if($location =~ //) {
		$has++;
		$ids{""} = $id;
		$locations{""} = $location;
	    }
	}
    }
}

foreach $id (@types){
    if(defined($ids{$id})) {
	printf "%15s %15s %s\n", $id, $ids{$id}, $locations{$id};
    }
    else {
	printf "%15s %15s\n", $id, "......";
    }
}
