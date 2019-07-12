Dir.glob($SCRIPT_DIR + '/lib/hash_functions/*.rb', &method(:require))

class BaseTest
  attr_accessor :hash_functions
  attr_reader :takes_input
  
  def initialize(params={})
    @ssdeep = SSDEEP.new("-s")
    @sise = SISE.new
    @sdhash = SDHASH.new
    @LZJD = LZJD.new
    @hash_functions = [@ssdeep, @sise, @sdhash, @LZJD]
    @index_iterator = []
    @max_size, @min_size = 100, 0
    @takes_input = true
  end
  
  def test(filenames=nil)
    reset_log
    filenames.each do |file|
      next if File.zero?(file)
      setup(file)
      while continue?
        process_file
        update_file_stats
      end
      teardown
    end
    return true
  end
  
  def results
    method = (respond_to? :set_method) ? set_method : ""
    headings = set_headings
    @rows = []
    @hash_functions.each_with_index do |h,i|
      log = h.log[@log_key]
      next if log.stats.empty?
      block_given? ? yield(log) : log.fill_results(@index_iterator.length)
      set_rows(h, log, i)
    end
    return as_error("No stats available.") if @rows.empty?
    table = Terminal::Table.new({
      title: set_title(method),
      headings: headings.flatten,
      rows: @rows,
      style: { border_x: '=', border_i: '.' }
    })
  end
  
  
  
  private
  
  def continue?
    @hash_functions.select{|x| x.is_active? }.any?
  end
  
  def reset_log
    @hash_functions.each { |h| h.log[@log_key].reset }    
  end
  
  def copy_file
    @copy = "copy_of_" + File.basename(@file)
    FileUtils.cp(@file, @copy)
  end
  
  def update_log
    @hash_functions.each do |h|
      h.reset
      h.log[@log_key].file[:name] = @file
      h.log[@log_key].file[:values] = {}
    end
  end
  
  def setup(file)
    @file = file
    @steps_done = 0
    reset_parameters if respond_to? :reset_parameters
    compute_chunk_size if respond_to? :compute_chunk_size
    copy_file
    update_log
  end

  def update_file_stats
    @steps_done += @step_size
    @hash_functions.select{|x| x.is_active? }.each do |h|
      h.compute_score(@file, @copy)
      h.log[@log_key].file[:values].merge!({$size_converter.number_to_human_size(@steps_done) => h.score}) if h.score >= 0
      h.deactivate if h.score <= 0
      @index_iterator << @steps_done
    end
  end
  
  
  def teardown
    @hash_functions.each { |h| h.log[@log_key].stats << h.log[@log_key].file.clone }
    @index_iterator.uniq!
    FileUtils.rm @copy if @copy
  end
    
  def set_rows(h, log, h_index)
    @rows << [[as_bold(h.class::PROGRAM_NAME)] << @index_iterator.map { |m| ""}].flatten
    @rows << [["Avg. Score"] << align_each_with_precision(log.means, PRECISION[:low])].flatten
    @rows << [["Matches"] << align_each(log.matches)].flatten
    @rows << [["Variance"] << align_each_with_precision(log.variance, PRECISION[:low])].flatten
    @rows << [["Std. deviation"] << align_each_with_precision(log.standard_deviation, PRECISION[:low])].flatten
    @rows << :separator if (h_index < @hash_functions.length-1)
  end
    
end
