require_relative 'fuzzy_hash_function'

class LZJD < FuzzyHashFunction
  
  PROGRAM_NAME = "LZJD.jar"

  def initialize(params="")
    super(params)
    @all_pair_comparison_command = PROGRAM_NAME + " -c"
    @gen_compare_command = PROGRAM_NAME + " -t -1 -g"
  end

  def self.file_stats(digest)
      if digest == ""
        info_hash = { filename: "", filesize: 0, digest: "", digest_length: 0}
      else
        info_hash = {}
        info_hash[:filename] = digest.split(':')[1] 
        info_hash[:filesize] = File.size info_hash[:filename]
	info_hash[:digest] = digest.split(':')[2]     
	info_hash[:digest_length] = self.digest_length(info_hash[:digest])
      end
      return info_hash 
  end
  
  def self.digest_length(digest)
    return 0 if digest.nil? or digest.empty?
    base64_part = digest.split(':')[-1]
    digest.length + (base64_part.length * 3 / 4) - base64_part.length
  end
  
  def self.split_match(match)
      parts = match.split('|') 
      left = File.basename(parts[0]).split('_')[0] 
      right = File.basename(parts[1]).split('_')[0] 
      score = parts[2].to_i 
      [left, right, score]
  end
end
