#!/bin/bash
#
# GitHub does not contain a full history of the Paleolatitude model because of
# licensing issues. The 'github' branch is an orphan branch based on the
# 'master' branch, which does not exist on GitHub.
#
# The rationale behind this set-up is: - full revision history still available
# off-site on 'master' (and other) branch - latest version publicly available
# on GitHub through 'github' branch - pull requests on 'github' branch can be
# processed through GitHub
#
# Unfortunately, this means that commits on 'master' can only be merged into
# the 'github' branch by cherry picking those commits (a regular merge or
# rebase would re-introduce the full history of 'master' on the 'github'
# branch). The two branches operate completely independently from eachother,
# with no common parents.
#
# This script attempts to solve this issue by cherry picking all commits from
# 'master' into 'github'

set -e

echo "Fetching latest data from remotes..."
git remote update > /dev/null
git checkout --quiet master > /dev/null

if [ ! "`git log HEAD..origin/master --oneline | wc -l`" = "0" ]; then
	echo "Your local branch 'master' is not up-to-date with 'origin/master'. Please fix that first. Aborting." >& 2
	exit 1
fi

# Ask Git which commits haven't been cherry picked from 'master' to 'github' yet:
tmp_unpicked_commits=`mktemp`
git cherry -v github HEAD github-orphan-diverge > "$tmp_unpicked_commits"

if [ "`cat $tmp_unpicked_commits | grep '^+' | wc -l`" = "0" ]; then
	# Not a single commit that can be picked
	echo "No cherry-pickable commits found. Aborting."
	rm "$tmp_unpicked_commits"
	exit 1
fi

echo "The commits indicated with a '+' have not been cherry-picked from 'master' into 'github' yet:"
cat "$tmp_unpicked_commits"

echo 
echo "Would you like to cherry pick this/these `grep '^+' "$tmp_unpicked_commits" | wc -l` commit(s) (y/n)? "

read -n1 response
echo 
echo 

pick_commits=`cat "$tmp_unpicked_commits" | grep '^+' | sed -n 's/.*\([0-9a-f]\{40\}\).*/\1/p' | tr '\n' ' '`
rm "$tmp_unpicked_commits"

if [ "$response" = "y" ]; then
	echo "Great. Hold on tight - handing you over to Git!"
	git checkout github
	git cherry-pick -x $pick_commits
else
	echo "Aborting."
	exit 1
fi

echo ""
echo "You are now on branch: `git rev-parse --abbrev-ref HEAD`. You might want to push your changes to GitHub?"
echo "Goodbye."
