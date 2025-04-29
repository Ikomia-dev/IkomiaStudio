for file in *png; do
	pngcheck "$file"
	pngcrush -rem iCCp -rem zTXt -rem tEXt -ow "$file"
done

