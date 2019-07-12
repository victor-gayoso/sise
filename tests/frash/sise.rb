require_relative 'fuzzy_hash_function' 

# This class inherits from BaseHashFunction.
#
# It sets the \ssdeep program that will be used and computes the digest length after parsing the output of the program.
class SISE < FuzzyHashFunction
	
  PROGRAM_NAME = "sise"

  def initialize(params="")
    super(params)
    @digest_header = "SiSe-1.0--7:1--blocksize:hash,filename\n"
    @all_pair_comparison_command = PROGRAM_NAME + " -x "
    @gen_compare_command = PROGRAM_NAME + " -c " 
  end

  def self.file_stats(digest)
      info_hash = {}
      info_hash[:filename] = digest.split(/,"/)[-1].rstrip.gsub(/"$/, '') 
      info_hash[:filesize] = File.size info_hash[:filename]
      chunk1 = digest.split(',"')[0...-1].first
      chunk2 = chunk1.split('::')
      info_hash[:digest] = chunk1
      info_hash[:digest_length] = self.digest_length(chunk2[0]).to_i + self.digest_length(chunk2[1]).to_i 
      return info_hash    
  end

  def self.digest_length(digest)
    return 0 if digest.nil? or digest.empty?
    parts = digest.split(':')
    base64_parts_length = parts[1..-1].inject(0) { |sum, b| sum + b.length }
    parts[0].length + (base64_parts_length * 3/4)
  end

  def self.split_match(match)
    parts = match.split(' ') 
    left = File.basename(parts[0]).split('_')[0] 
    right = File.basename(parts[2]).split('_')[0] 
    score = parts[3][1...-1].to_i 
    [left,right,score]
  end
end
